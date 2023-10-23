/*
 *
 *
 *    decode.h
 *
 *    Decode task module.
 *
 *    Copyright (C) 2023 by Matt Roberts.
 *    License: GNU GPL3 (www.gnu.org)
 *
 *
 */


#ifndef __KK5JY_FT8_DECODE_H
#define __KK5JY_FT8_DECODE_H

// C++ STL types
#include <string>
#include <deque>

// for popen(...) and file I/O
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

// for WAV file interface
#include "sf.h"

// string operations
#include "stype.h"

// clock operations
#include "clock.h"

// for chmod(...)
#include <sys/stat.h>

// for DEBUG only
#ifdef VERBOSE_DEBUG
#include <iostream>
#include <typeinfo>
#endif

namespace KK5JY {
	namespace FT8 {
		// the worker thread
		void *decoder_thread(void *parent);

		//
		//  class DecodeBase
		//
		class DecodeBase {
			protected:
				// common/shared data elements
				std::string m_Mode;
				std::string m_Path;
				std::deque<std::string> m_Buffer;
				double m_DecodeStartTime;
				double m_CaptureStartTime;
				size_t m_Samples;
				volatile short m_Depth;
				volatile bool m_Done;

				const size_t JT9_RATE = 12000;

				// allow thread worker to access private data
				friend void *decoder_thread(void *parent);

			public:
				DecodeBase() : m_Depth(1), m_Done(false) { /* nop */ }
				virtual ~DecodeBase() { /* nop */ }

				double GetDecodeStart() const { return m_DecodeStartTime; }
				double GetCaptureStart() const { return m_CaptureStartTime; }
		};


		//
		//  class Decode<T>
		//
		template <typename T>
		class Decode : public DecodeBase {
			private:
				SoundFile *m_WAV;

			public:
				Decode(
					const std::string &mode,
					const std::string &wav_path,
					double start,
					short depth = 2);
				virtual ~Decode();

			public:
				// add more WAV data to be decoded
				size_t write(T* buffer, size_t count);

				// close the WAV file and start the decoding process
				bool startDecode();

				// copy the decodes into the buffer provided
				size_t getDecodes(std::deque<std::string> &buffer);

				// returns true iff the decoder is finished
				bool isDone() const volatile { return m_Done; }
		};


		template <typename T>
		inline Decode<T>::Decode(
				const std::string &mode,
				const std::string &wav_path,
				double start,
				short depth) {
			// store the file name and start time
			m_Mode = my::strip(my::toLower(mode));
			m_Path = wav_path;
			m_CaptureStartTime = start;
			m_DecodeStartTime = 0;
			m_Depth = depth;
			m_Samples = 0;

			// open the sound file
			m_WAV = new SoundFile(wav_path, JT9_RATE, 1,
                SoundFile::major_formats::wav,
				SoundFile::minor_formats::s16);
			::chmod(wav_path.c_str(), 0600); // only owner can r/w the WAV file
		}


		template <typename T>
		inline Decode<T>::~Decode() {
			if (m_WAV) {
				SoundFile *toDelete = m_WAV;
				toDelete->close();
				delete toDelete;
				m_WAV = 0;
			}
		}


		template <typename T>
		inline size_t Decode<T>::write(T* buffer, size_t count) {
			// sanity checks
			if (m_Done || ! m_WAV)
				return 0;

			// update the sample counter
			m_Samples += count;

			// write data to the file
			return m_WAV->write(buffer, count);
		}


		template <typename T>
		inline bool Decode<T>::startDecode() {
			if ( ! m_WAV) {
				return false;
			}

			m_DecodeStartTime = abstime();

			// pad the WAV file if needed to make them all the same length
			size_t full_frame = 0;
			if (m_Mode == "ft8")
				full_frame = 13.5 * JT9_RATE;
			else if (m_Mode == "ft4")
				full_frame = 6.5 * JT9_RATE;
			for (size_t i = m_Samples; i < full_frame; ++i) {
				T sample = 0;
				m_WAV->write(sample);
			}

			// close the wave file
			SoundFile *toDelete = m_WAV;
			toDelete->close();
			delete toDelete;
			m_WAV = 0;
			m_Samples = 0;

			// build new thread attributes
			::pthread_attr_t attrs;
			pthread_attr_init(&attrs);
			pthread_attr_setdetachstate(&attrs, PTHREAD_CREATE_DETACHED);

			// start the thread
			pthread_t id;
			int result = pthread_create(&id, &attrs, decoder_thread, dynamic_cast<DecodeBase*>(this));

			// return result
			return id != 0 && result == 0;
		}


		template <typename T>
		inline size_t Decode<T>::getDecodes(std::deque<std::string> &buffer) {
			if ( ! m_Done)
				return 0;

			// copy from our buffer into the caller's
			size_t result = 0;
			std::deque<std::string>::const_iterator i;
			for (i = m_Buffer.begin(); i != m_Buffer.end(); ++i) {
				buffer.push_back(*i);
				++result;
			}

			// clear the local buffer so we can't fetch it again
			m_Buffer.clear();

			// return number of items copied
			return result;
		}


		//
		//  the worker thread
		//
		inline void *decoder_thread(void *parent) {
			#ifdef VERBOSE_DEBUG
			std::cerr << "thread start" << std::endl;
			#endif

			DecodeBase *decode = reinterpret_cast<DecodeBase*>(parent);
			if ( ! decode)
				pthread_exit(0);

			try {
				#ifdef VERBOSE_DEBUG
				std::cerr << "try" << std::endl;
				std::cerr << "file is " << decode->m_Path << std::endl;
				#endif

				// start 'jt9' on the temp file
				std::string cmd = "jt9";
				if (decode->m_Mode == "ft8")
					cmd += " --ft8 ";
				else
					cmd += " --ft4 ";
				cmd += " -d ";
				cmd += (char)(decode->m_Depth + '0');
				cmd += ' ';
				cmd += decode->m_Path;
				FILE *jt9 = popen(cmd.c_str(), "r");

				#ifdef VERBOSE_DEBUG
				std::cerr << "popen(" << cmd << ")" << std::endl;
				if (jt9)
					std::cerr << "jt9 started" << std::endl;
				#endif

				// I/O loop on 'jt9' output
				char iobuffer[128];
				std::string linebuffer;
				while ( ! feof(jt9)) {
					int ct = fread(iobuffer, 1, 128, jt9);
					if (ct <= 0)
						break;

					#ifdef VERBOSE_DEBUG
					std::cerr << "jt9 sent " << ct << " bytes" << std::endl;
					#endif

					// process the new data
					linebuffer.append(iobuffer, ct);
					std::string::size_type idx = linebuffer.find('\n');
					while (idx != std::string::npos) {
						std::string line = my::strip(linebuffer.substr(0, idx));
						if (isdigit(line[0]) && isdigit(line[1]))
							decode->m_Buffer.push_back(line);
						linebuffer = linebuffer.substr(idx + 1);
						idx = linebuffer.find('\n');
					}
				}

				// close the pipe to the child
				fclose(jt9);

				// make sure to include remainder
				std::string::size_type idx = linebuffer.find('\n');
				while (idx != std::string::npos) {
					std::string line = my::strip(linebuffer.substr(0, idx));
					if (isdigit(line[0]) && isdigit(line[1]))
						decode->m_Buffer.push_back(line);
					linebuffer = linebuffer.substr(idx + 1);
					idx = linebuffer.find('\n');
				}
			} catch (const std::exception &ex) {
				#ifdef VERBOSE_DEBUG
				std::cerr << "caught exception: " << ex.what() << std::endl;
				#endif
			}

			#ifdef VERBOSE_DEBUG
			std::cerr << "thread complete" << std::endl;
			#endif

			// all done
			unlink(decode->m_Path.c_str());
			decode->m_Done = true;
			pthread_exit(0);
		}
	}
}

#endif // __KK5JY_FT8_DECODE_H
