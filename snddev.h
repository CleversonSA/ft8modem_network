/*
 *
 *
 *    snddev.h
 *
 *    Sound interface and decoding framework.
 *
 *    Copyright (C) 2023 by Matt Roberts.
 *    License: GNU GPL3 (www.gnu.org)
 *
 *
 */

#ifndef __KK5JY_FT8MODEM_H
#define __KK5JY_FT8MODEM_H

// C++ includes
#include <iostream>
#include <stdexcept>

// other stuff
#include <vector>
using std::vector;

#include <ctype.h>
#include <cstring>
using std::strcat;
using std::strlen;
using std::strcpy;
using std::string;

#include <cstdio>
using std::sprintf;

// C math module
#include <cmath>

// sound card base class
#include "sc.h"

// modulator
#include "mfsk.h"

// demod/decoder interface
#include "decode.h"

// encoding support
#include "encode.h"

// clock helper
#include "clock.h"

// modulator
#include "mfsk.h"

// FIR filter
#include "FirFilter.h"

// mutex locking
#include "locker.h"


//
//  enum TimeSlots
//
enum TimeSlots {
	NextSlot = 0,
	OddSlot = 1,
	EvenSlot = 2
};


//
//  Decoded text line handler
//	@Author: CleversonSA
//
class DecodedLine {
	private:
		long int _time;
		string _content;
	
	public:
		DecodedLine(long int = 0, string = "");
		~DecodedLine();
		void setTime(long int);
		long int getTime() const;
		string getContent() const;
};


inline DecodedLine::DecodedLine(long int time, string content):
	_time(time),
	_content(content)
{

}

inline DecodedLine::~DecodedLine()
{

}

inline void DecodedLine::setTime(long int time)
{
	_time = time;
}

inline long int DecodedLine::getTime() const
{
	return (_time);
}

inline string DecodedLine::getContent() const
{
	return (_content);
}


//
//  ModemSoundDevice
//
class ModemSoundDevice : public SoundCard {
	private:
		// the clock
		KK5JY::FT8::FrameClock m_Clock;

		// the decimator clock
		KK5JY::DSP::FirFilter<float> *m_Filter;

		// decoders
		KK5JY::FT8::Decode<float> *m_Current;
		KK5JY::FT8::Decode<float> *m_Decoding;

		// the modulator
		KK5JY::DSP::MFSK::Modulator<float> *m_MFSK;

		std::string m_TempDir; // path to temp folder
		std::string m_Mode;  // mode string
		size_t m_Rate;     // sampling ratevoid
		size_t m_DecFact;  // decimation factor
		size_t m_Lead;     // number of samples of silence to emit before transmission
		double m_FrameStart, m_FrameEnd, m_FrameSize, m_TxWinStart, m_TxWinEnd;
		double m_bps, m_shift; // MFSK parameters
		short m_Depth; // decoding depth (1...3)
		float m_Volume; // output volume (normalized)
		volatile bool m_FrameCounter;
		volatile bool m_Sending;
		volatile bool m_Active;
		volatile bool m_Abort;

		enum TimeSlots m_Slot;

		// critical section mutex
		my::mutex m_Mutex;

	public:
		ModemSoundDevice(const std::string &mode, size_t id, size_t rate, size_t win = 512);
		~ModemSoundDevice();

		// TODO: this should probably be replace by a thread
		vector<DecodedLine> * run();

		// send a message
		bool transmit(const std::string &message, double f0, TimeSlots slot = NextSlot);

		// stop sending immediately
		void cancelTransmit();

		// test whether sound card is running
		bool isActive() const volatile { return m_Active; }

		// set the decoding depth
		short setDepth(short depth);

		// get the decoding depth
		short setDepth(void) const { return m_Depth; }

		// set the lead-in silence (samples)
		size_t setLead(size_t newVal) { return (m_Lead = newVal); }

		// get the lead-in silence (samples)
		size_t getLead(void) const { return m_Lead; }

		// set the volume (normalized)
		float setVolume(float newVal) { return (m_Volume = newVal); }

		// get the volume (normalized)
		float getVolume(void) const { return m_Volume; }

	protected:
		void event(float *in, float *out, size_t count);
};


//KK5JY
//  ModemSoundDevice::ctor
//
inline ModemSoundDevice::ModemSoundDevice(const std::string &mode, size_t id, size_t rate, size_t win) :
		SoundCard(id, rate, 1, win),
		m_Filter(0), m_Current(0), m_Decoding(0), m_MFSK(0) {
	m_Mode = mode;
	m_TempDir = "/tmp/"; // TODO: make this configurable
	m_Depth = 1;
	m_Rate = rate;
	m_FrameCounter = false;
	m_Sending = false;
	m_Lead = 0.125 * m_Rate; // 125ms
	m_Volume = 0.5; // 50%
	m_Abort = false;
	// calculate the decimation factor
	m_DecFact = rate / 12000;

	// make sure the values work
	if (rate % 12000) {
		throw std::runtime_error("Sampling rate must be multiple of 12000Hz");
	}
	if (win % m_DecFact) {
		throw std::runtime_error("Window size must be multiple of decimation factor");
	}

	// allocate decimation filter
	m_Filter = new KK5JY::DSP::FirFilter<float>(
		KK5JY::DSP::FirFilterTypes::LowPass, // type
		25,    // taps
		5000,  // cutoff
		rate); // rate

	// configure mode-specific timings
	std::string realMode = my::strip(my::toUpper(mode));
	if (realMode == "FT8") {
		m_TxWinStart = 0.0;
		m_TxWinEnd = 2.0;
		m_FrameSize = 15.0; // the slot length
		m_FrameStart = 14.9;
		m_FrameEnd = 13.0;
		m_bps = 6.25;
		m_shift = m_bps;
	} else if (realMode == "FT4") {
		m_TxWinStart = 0.0;
		m_TxWinEnd = 1.0;
		m_FrameSize = 15.0 / 2; // the slot length
		m_FrameStart = 14.9 / 2;
		m_FrameEnd = 13.0 / 2;
		m_bps = 12000.0 / 576.0;
		m_shift = m_bps;
	} else {
		throw std::runtime_error("Unsupported mode provided");
	}
}

//
//  ModemSoundDevice::dtor
//
inline ModemSoundDevice::~ModemSoundDevice() {
	if (m_Filter)
		delete m_Filter;
}

//
//  ModemSoundDevice::setDepth(...)
//
inline short ModemSoundDevice::setDepth(short depth) {
	if (depth >= 1 && depth <= 3) {
		m_Depth = depth;
	}
	return m_Depth;;
}

//
//  ModemSoundDevice::run()
//
inline vector<DecodedLine> * ModemSoundDevice::run() {
	KK5JY::FT8::Decode<float> *decoding = m_Decoding;

	vector<DecodedLine> * decodedLinesVectorPtr = new vector<DecodedLine>;
	
	DecodedLine *dlPtr = 0;

	if (decoding) {

		std::deque<std::string> buffer;
		if (decoding->isDone()) {
			// fetch the decodes
			decoding->getDecodes(buffer);
			double when = ::ceil(m_Decoding->GetCaptureStart());
			delete m_Decoding;

			// and print them
			m_Decoding = 0;
			std::deque<std::string>::const_iterator i;
			int count = 0;
			for (i = buffer.begin(); i != buffer.end(); ++i) {
				// std::cout << "D: " <<  << " " << (i->substr(7)) << std::endl;
				//std::cout << static_cast<time_t>(when) << ";" <<(i->substr(7)) << std::endl;
				dlPtr = new DecodedLine(static_cast<time_t>(when), (i->substr(7)));
				(*decodedLinesVectorPtr).push_back((*dlPtr));
				count ++;
			}
			
			// talk
			// std::cout << "INFO: Decode cycle complete." << std::endl;
			//std::cout.flush();
		}
		
	}

	return decodedLinesVectorPtr;
}


//
//  ModemSoundDevice::transmit(...)
//
inline bool ModemSoundDevice::transmit(const std::string &message, double f0, TimeSlots slot) {
	// encode to keying symbols
	std::string linebuffer = KK5JY::FT8::encode(m_Mode, message);

	// store the TX slot
	m_Slot = slot;

	// lock critical section from here to end of function
	my::locker lock(m_Mutex);

	// either update or start a new modulator for this message
	if ( ! m_MFSK) {
		// queue a new modulator to handle the message
		m_MFSK = new KK5JY::DSP::MFSK::Modulator<float>(
			m_Rate, f0, m_bps, m_shift);
		m_MFSK->setLead(m_Lead);
		m_MFSK->setVolume(m_Volume);
	}

	// update the message in the modulator
	m_MFSK->transmit(linebuffer, f0);

	// success
	return true;
}


//
//  ModemSoundDevice::cancelTransmit
//
inline void ModemSoundDevice::cancelTransmit() {
	// lock critical section from here to end of function
	my::locker lock(m_Mutex);

	m_Abort = true; // tell the event handler to stop and clean up
}


//
//  ModemSoundDevice::event - sound card event handler
//
inline void ModemSoundDevice::event(float *in, float *out, size_t count) {
	// read the frame clock
	double sec = m_Clock.seconds(m_FrameSize);

	// set active flag
	if (count) m_Active = true;

	//
	//  RECEIVER: feed the current decoder
	//
	KK5JY::FT8::Decode<float> *decoder = m_Current;
	if (decoder) {
		if (m_Rate == 12000) {
			// copy data into decode module
			if ( ! m_Sending)
				decoder->write(in, count);
		} else {
			// run decimation filter across the input
			float *fp = in;
			const float * const ep = in + count;
			while (fp++ != ep) {
				*fp = m_Filter->run(*fp);
			}

			// decimate the input
			for (size_t i = 1; i != count / m_DecFact; ++i) {
				in[i] = in[i * m_DecFact];
			}

			// copy data into decode module
			if ( ! m_Sending)
				decoder->write(in, count / m_DecFact);
		}

		// if frame ended, move current deocder to 'decoding' state
		if (sec > m_FrameEnd && sec < m_FrameStart) {
			#ifdef VERBOSE_DEBUG
			std::cerr << sec << ": End decode capture." << std::endl;
			#endif

			m_Decoding = m_Current;
			m_Current = 0;

			// and start it decoding
			decoder->startDecode();
		}
	} else {
		if (sec >= m_FrameStart || sec < m_FrameEnd) {
			#ifdef VERBOSE_DEBUG
			std::cerr << sec << ": Start decode capture; frame counter = " << m_FrameCounter << std::endl;
			#endif

			std::string name = m_TempDir + "100000_000000.wav";
			if (m_FrameCounter) {
				name[9] = '1';
			}
			m_FrameCounter = ! m_FrameCounter;
			m_Current = new KK5JY::FT8::Decode<float>(m_Mode, name, KK5JY::FT8::abstime(), m_Depth);
		}
	}


	//
	//  TRANSMITTER - if a message is pending, send it at the next slot
	//

	// lock critical section from here to end of function
	my::locker lock(m_Mutex);

	// if ready to transmit, but not yet sending, and at the start of the,
	//    frame time, enable the transmitter
	bool inWindow = ( ! m_Sending) && m_MFSK && (sec > m_TxWinStart) && (sec < m_TxWinEnd);
	if (inWindow) {
		bool thisSlot = ! m_Slot; // if NextSlot selected, transmit now
		if ( ! thisSlot) {
			int slot_num = static_cast<int>(m_Clock.seconds(60) / m_FrameSize);
			thisSlot |= ((m_Slot == OddSlot) && (slot_num % 2));  // in odd window
			thisSlot |= ((m_Slot == EvenSlot) && ! (slot_num % 2)); // in even window

			// DEBUG: very verbose output
			std::cout
				<< "TRACE: SlotTarget = " << m_Slot
				<< "; SlotNow = " << slot_num
				<< "; sec = " << m_Clock.seconds(60)
				<< "; thisSlot = " << thisSlot << std::endl;
		}
		if (thisSlot) {
			std::cout << "TX: 1" << std::endl;
			std::cout << "INFO: Enable modulator." << std::endl;
			std::cout.flush();

			m_Sending = true;
		}
	}

	// if already sending, keep going
	if (m_Sending) {
		// read data right into the I/O buffer
		size_t ct = m_MFSK->read(out, count);

		// if data exhausted, shut down modulator
		if (m_Abort || ! ct) {
			std::cout << "TX: 0" << std::endl;
			std::cout << "INFO: Disable modulator." << std::endl;
			std::cout.flush();

			m_Sending = false;
			m_Abort = false;
			delete m_MFSK;
			m_MFSK = 0;
		}

		// zero out the rest of the memory
		float *ep = out + count;
		float *zp = out + ct;
		while (zp != ep) {
			*zp++ = 0;
		}
	} else {
		// zero out the TX buffer to get silence
		float *ep = out + count;
		float *zp = out;
		while (zp != ep) {
			*zp++ = 0;
		}
	}
}

#endif // __KK5JY_FT8MODEM_H
