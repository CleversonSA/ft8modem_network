/*
 *
 *
 *    mfsk.h
 *
 *    General-purpose MFSK modulator.
 *
 *    Copyright (C) 2023 by Matt Roberts.
 *    License: GNU GPL3 (www.gnu.org)
 *
 *
 */


#ifndef __KK5JY_DSP_MFSK_H
#define __KK5JY_DSP_MFSK_H

#include "shape.h"
#include "osc.h"
#include "es.h"

// number of cycles to shape on each end
#define KK5JY_MFSK_SHAPER_CYCLES (10)


namespace KK5JY {
	namespace DSP {
		namespace MFSK {
			//
			//  class Modulator<T>
			//
			template <typename T>
			class Modulator {
				private:
					// samples-per-bit ratio
					const uint16_t bit_ratio;

					const double m_fs; // the sampling rate
					double m_f0;    // the lowest tone in the group
					double m_bps;   // transmitted symbol rate (per second)
					double m_shift; // frequency shift between adjacent tones
					std::string m_msg; // the message symbols
					size_t m_idx;   // current symbol index
					size_t m_ctr;   // number of samples emitted for current symbol
					size_t m_lead;  // how much silence to emit up front
					size_t m_leadctr; // lead-in counter
					T m_Volume;     // output volume

					// the oscillator
					Osc<T> m_osc;

					// the shift-shaper; smooths transition between symbols
					Smoother<T> m_lpf;

					// the envelope shaper; used to generate ramp-in and -out
					Shaper<T> m_A;

				public:
					Modulator(
						double fs,     // sampling frequency
						double f0,     // lowest tone frequency
						double bps,    // symbols per second
						double shift); // shift between adjacent tones);
				
				public:
					// send a message; optionally change the lowest tone frequency
					void transmit(const std::string &message, double f0 = 0);

					// reset the modulator state
					void clear();

					// reads out the encode wave data
					size_t read(T* buffer, size_t count);

					// set the lead-in silence (samples)
					size_t setLead(size_t newVal) { return (m_lead = newVal); }

					// get the lead-in silence (samples)
					size_t getLead(void) const { return m_lead; }

					// set the volume (normalized)
					T setVolume(T newVal) { return (m_Volume = newVal); }

					// get the volume (normalized)
					T getVolume(void) const { return m_Volume; }
			};


			//
			//  Modulator<T>::Modulator
			//
			template <typename T>
			inline Modulator<T>::Modulator(
				double fs, double f0, double bps, double shift) :
					bit_ratio(round(
						static_cast<double>(fs) /
						static_cast<double>(bps))),
					m_fs(fs), m_f0(f0), m_bps(bps), m_shift(shift),
					m_osc(f0, fs), m_lpf(LowpassToAlpha(fs, bps), f0),
					m_A(KK5JY_MFSK_SHAPER_CYCLES * fs / f0) {
				m_lead = m_fs / 8; // 0.125 s
				m_Volume = 0.9; // 90%
				clear();
			}


			//
			//  Modulator<T>::transmit(...)
			//
			template <typename T>
			inline void Modulator<T>::transmit(const std::string &message, double f0) {
				if (f0 > 0) {
					m_f0 = f0;
				}
				std::string clean;
				clean.reserve(message.size());
				std::string::const_iterator i;
				for (i = message.begin(); i != message.end(); ++i) {
					char ch = *i;
					if (isdigit(ch))
						clean += ch;
				}
				m_msg = clean;
				if (m_ctr == 0)
					m_leadctr = 0;
			}


			//
			//  Modulator<T>::clear()
			//
			template <typename T>
			inline void Modulator<T>::clear() {
				m_msg = "";
				m_idx = 0;
				m_ctr = 0;
				m_leadctr = 0;
			}


			//
			//  Modulator<T>::read(...)
			//
			template <typename T>
			inline size_t Modulator<T>::read(T* buffer, size_t count) {
				if (m_msg.size() == 0)
					return 0;

				// lead-in silence generation
				size_t samples = 0;
				while (m_leadctr < m_lead && samples < count) {
					*buffer++ = 0;
					++m_leadctr;
					++samples;
				}

				// if the lead-in silence consumed the whole buffer, quit
				if (samples == count)
					return count;

				// fetch the current symbol
				char ch = m_msg[m_idx];

				// calculate current output frequency
				double f = m_f0 + (m_shift * (ch - '0'));

				// main modulator loop
				do {
					// compute the unfiltered envelope value
					bool lead_out =
						(m_idx == (m_msg.size() - 1)) &&
						(m_ctr >= static_cast<size_t>(bit_ratio - m_A.size()));
					double A_in = lead_out ? 0 : 1;

					// generate the waveform; add lead-in/out where needed
					m_osc.setFreq(m_lpf.run(f), m_fs);
					*buffer++ = m_osc.read() * m_Volume * m_A.run(A_in);
					//             ^ LO ^    *  ^vol^   *   ^env^

					// if time for the next symbol...
					if (++m_ctr == bit_ratio) {
						m_ctr = 0;

						// if everything sent, stop now
						if (++m_idx == m_msg.size()) {
							clear();
							break;
						}

						// fetch next character
						ch = m_msg[m_idx];

						// compute the new frequency
						f = m_f0 + (m_shift * (ch - '0'));
					}
				} while (++samples != count);

				// return the number of samples written to 'buffer'
				return samples;
			}
		}
	}
}

#endif // __KK5JY_DSP_MFSK_H
