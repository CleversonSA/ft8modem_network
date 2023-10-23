/*
 *
 *
 *    FirFilter.h
 *
 *    Filtering classes; translated from C#
 *
 *    Copyright (C) 2013-2021 by Matt Roberts, KK5JY.
 *    All rights reserved.
 *
 *    License: GNU GPL3 (www.gnu.org)
 *    
 *
 */

#ifndef __KK5JY_FIRFILTER_H
#define __KK5JY_FIRFILTER_H

#include <cmath>
#include <cstdlib>
#include <exception>
#include <string>
#include <algorithm>

#include "WindowFunctions.h"
#include "FilterTypes.h"
#include "FilterUtils.h"
#include "IFilter.h"
#include "nlimits.h"

// for debugging only
//#define VERBOSE_DEBUG
#ifdef VERBOSE_DEBUG
#include <iostream>
#endif

namespace KK5JY {
	namespace DSP {
		#ifndef local_min
		#define local_min(a, b) std::min(a, b)
		#endif

		// define specific function pointer types
		typedef double (*WindowFunction)(int n, int N);
		typedef double (*CoefFunction1)(double d, int i, int N);
		typedef double (*CoefFunction2)(double d1, double d2, int i, int N);

		//
		//  Exception class for reporting errors from the filter configuration
		//
		class FirFilterException : public std::exception {
			private:
				std::string userMsg;
			public:
				FirFilterException(const std::string &s) : userMsg(s) { /* nop */ }
				~FirFilterException() throw() { /* nop */ }
				const char *what() const throw() { return userMsg.c_str(); }
		};


		//
		//  Utilities for computing filter coefficients, and generic filter implementation
		//
		class FirFilterUtils {
			public:
				/// <summary>
				/// Generate low-pass filter coefficients for an odd-length filter of length N.
				/// </summary>
				/// <param name="omega_c">Normalized cutoff frequency.</param>
				/// <param name="n">The sample number.</param>
				/// <param name="N">The window length.</param>
				static double IdealLowPass(double omega_c, int n, int N) {
					if (n == 0) {
						return omega_c / M_PI;
					} else {
						return sin(omega_c * n) / (M_PI * n);
					}
				}

				/// <summary>
				/// Generate high-pass filter coefficients for an odd-length filter of length N.
				/// </summary>
				/// <param name="omega_c">Normalized cutoff frequency.</param>
				/// <param name="n">The sample number.</param>
				/// <param name="N">The window length.</param>
				static double IdealHighPass(double omega_c, int n, int N) {
					if (n == 0) {
						return 1.0 - (omega_c / M_PI);
					} else {
						return -1.0 * sin(omega_c * n) / (M_PI * n);
					}
				}

				/// <summary>
				/// Generate resonant filter coefficients for an odd-length filter of length N.
				/// </summary>
				/// <param name="omega_c">Normalized center frequency.</param>
				/// <param name="n">The sample number.</param>
				/// <param name="N">The window length.</param>
				static double IdealResonant(double omega_c, int n, int N) {
					return cos(omega_c * n) / (N / M_PI);
				}

				/// <summary>
				/// Generate band-pass filter coefficients for an odd-length filter of length N.
				/// </summary>
				/// <param name="omega_1">Normalized lower cutoff frequency.</param>
				/// <param name="omega_2">Normalized upper cutoff frequency.</param>
				/// <param name="n">The sample number.</param>
				/// <param name="N">The window length.</param>
				static double IdealBandPass(double omega_1, double omega_2, int n, int N) {
					if (n == 0) {
						return (omega_2 - omega_1) / M_PI;
					} else {
						return (sin(omega_2 * n) / (M_PI * n)) - (sin(omega_1 * n) / (M_PI * n));
					}
				}

				/// <summary>
				/// Generate band-stop filter coefficients for an odd-length filter of length N.
				/// </summary>
				/// <param name="omega_1">Normalized lower cutoff frequency.</param>
				/// <param name="omega_2">Normalized upper cutoff frequency.</param>
				/// <param name="n">The sample number.</param>
				/// <param name="N">The window length.</param>
				static double IdealBandStop(double omega_1, double omega_2, int n, int N) {
					if (n == 0) {
						return 1.0 - ((omega_2 - omega_1) / M_PI);
					} else {
						return (sin(omega_1 * n) / (M_PI * n)) - (sin(omega_2 * n) / (M_PI * n));
					}
				}

				/// <summary>
				/// Generate twin-peak filter coefficients for an odd-length filter of length N.
				/// </summary>
				/// <param name="omega_1">Normalized first peak cutoff frequency.</param>
				/// <param name="omega_2">Normalized second peak cutoff frequency.</param>
				/// <param name="n">The sample number.</param>
				/// <param name="N">The window length.</param>
				static double IdealTwinPeak(double omega_1, double omega_2, int n, int N) {
					return
						(
						 FirFilterUtils::IdealResonant(omega_1, n, N) +
						 FirFilterUtils::IdealResonant(omega_2, n, N)
						);
				}

			private:
				// Generate windowed coefficients for single-cutoff filters
				static double *GenerateCoefficients(WindowFunction f, CoefFunction1 g, int length, double omega_c) {
					int limit = length / 2;
					double* result = new double[length];
					for (int i = -limit; i <= limit; ++i) {
						double wn = f(i, length);
						double hn = g(omega_c, i, length);
						result[i + limit] = (wn * hn);
						#ifdef VERBOSE_DEBUG
						std::cerr << "h[" << i << "]: " << hn << "; w[" << i << "]: " << wn << "; coef[" << i + limit << "]: " << result[i + limit] << std::endl;
						#endif
					}
					return result;
				}

				// Generate windowed coefficients for single-cutoff filters
				static double *GenerateCoefficients(WindowFunction f, CoefFunction2 g, int length, double omega_c1, double omega_c2) {
					int limit = length / 2;
					double* result = new double[length];
					for (int i = -limit; i <= limit; ++i) {
						double wn = f(i, length);
						double hn = g(omega_c1, omega_c2, i, length);
						result[i + limit] = (wn * hn);
						#ifdef VERBOSE_DEBUG
						std::cerr << "h[" << i << "]: " << hn << "; w[" << i << "]: " << wn << "; coef[" << i + limit << "]: " << result[i + limit] << std::endl;
						#endif
					}
					return result;
				}

				// convert double array to float array
				template <typename sample_t>
				static sample_t *fromDouble(double *d, size_t len) {
					sample_t *result = new sample_t[len];
					for (size_t i = 0; i != len; ++i) {
						result[i] = d[i] * norm_limits<sample_t>::maximum;
					}
					delete[] d;
					return result;
				}

			public:
				template <typename T>
				static T* GenerateLowPassCoefficients(WindowFunction wf, int length, double omega_c);
				template <typename T>
				static T* GenerateHighPassCoefficients(WindowFunction wf, int length, double omega_c);
				template <typename T>
				static T* GenerateResonantCoefficients(WindowFunction wf, int length, double omega_c);
				template <typename T>
				static T* GenerateBandPassCoefficients(WindowFunction wf, int length, double omega_c1, double omega_c2);
				template <typename T>
				static T* GenerateTwinPeakCoefficients(WindowFunction wf, int length, double omega_c1, double omega_c2);
				template <typename T>
				static T* GenerateBandStopCoefficients(WindowFunction wf, int length, double omega_c1, double omega_c2);

			public:
				//
				// FIR filter core (int16).
				//
				static int16_t Filter(int16_t input, int16_t *&hp, int16_t * const history, int16_t * const hist_end, int16_t * const coefs, int16_t * const coef_end) {
					int16_t* cp = coefs;

					// store the input sample and wrap the history pointer
					*hp++ = input;
					if (hp == hist_end) {
						hp = history;
					}

					// run the filter
					int16_t output = 0;
					while (cp != coef_end) {
						// multiply and accumulate
						output += (*hp++ * *cp++) >> 15;

						// and make sure to wrap the history pointer
						if (hp == hist_end) {
							hp = history;
						}
					}

					// return the result
					return output;
				}

				//
				// FIR filter core (single precision).
				//
				static float Filter(float input, float *&hp, float * const history, float * const hist_end, float * const coefs, float * const coef_end) {
					float* cp = coefs;

					// store the input sample and wrap the history pointer
					*hp++ = input;
					if (hp == hist_end) {
						hp = history;
					}

					// run the filter
					float output = 0.0f;
					while (cp != coef_end) {
						// multiply and accumulate
						output += *hp++ * *cp++;

						// and make sure to wrap the history pointer
						if (hp == hist_end) {
							hp = history;
						}
					}

					// return the result
					return output;
				}

				//
				// FIR filter core (double precision).
				//
				static double Filter(double input, double *&hp, double * const history, double * const hist_end, double * const coefs, double * const coef_end) {
					double* cp = coefs;

					// store the input sample and wrap the history pointer
					*hp++ = input;
					if (hp == hist_end) {
						hp = history;
					}

					// run the filter
					double output = 0.0;
					while (cp != coef_end) {
						// multiply and accumulate
						output += *hp++ * *cp++;

						// and make sure to wrap the history pointer
						if (hp == hist_end) {
							hp = history;
						}
					}

					// return the result
					return output;
				}
		};


		///
		/// Generate windowed low-pass coefficients (int16).
		///
		template <>
		inline int16_t* FirFilterUtils::GenerateLowPassCoefficients<int16_t>(WindowFunction f, int length, double omega_c) {
			return fromDouble<int16_t>(GenerateCoefficients(f, IdealLowPass, length, omega_c), length);
		}

		///
		/// Generate windowed high-pass coefficients (int16).
		///
		template <>
		inline int16_t* FirFilterUtils::GenerateHighPassCoefficients<int16_t>(WindowFunction f, int length, double omega_c) {
			return fromDouble<int16_t>(GenerateCoefficients(f, IdealHighPass, length, omega_c), length);
		}

		///
		/// Generate windowed resonant coefficients (int16).
		///
		template<>
		inline int16_t* FirFilterUtils::GenerateResonantCoefficients<int16_t>(WindowFunction f, int length, double omega_c) {
			return fromDouble<int16_t>(GenerateCoefficients(f, IdealResonant, length, omega_c), length);
		}

		///
		/// Generate windowed twin-peak coefficients (int16).
		///
		template<>
		inline int16_t* FirFilterUtils::GenerateTwinPeakCoefficients<int16_t>(WindowFunction f, int length, double omega_c1, double omega_c2) {
			return fromDouble<int16_t>(GenerateCoefficients(f, IdealTwinPeak, length, omega_c1, omega_c2), length);
		}

		///
		/// Generate windowed band-pass coefficients (int16).
		///
		template <>
		inline int16_t* FirFilterUtils::GenerateBandPassCoefficients<int16_t>(WindowFunction f, int length, double omega_c1, double omega_c2) {
			return fromDouble<int16_t>(GenerateCoefficients(f, IdealBandPass, length, omega_c1, omega_c2), length);
		}

		///
		/// Generate windowed band-stop coefficients (int16).
		///
		template <>
		inline int16_t* FirFilterUtils::GenerateBandStopCoefficients<int16_t>(WindowFunction f, int length, double omega_c1, double omega_c2) {
			return fromDouble<int16_t>(GenerateCoefficients(f, IdealBandStop, length, omega_c1, omega_c2), length);
		}


		///
		/// Generate windowed low-pass coefficients (double precision).
		///
		template <>
		inline double* FirFilterUtils::GenerateLowPassCoefficients<double>(WindowFunction f, int length, double omega_c) {
			return GenerateCoefficients(f, IdealLowPass, length, omega_c);
		}

		///
		/// Generate windowed high-pass coefficients (double precision).
		///
		template <>
		inline double* FirFilterUtils::GenerateHighPassCoefficients<double>(WindowFunction f, int length, double omega_c) {
			return GenerateCoefficients(f, IdealHighPass, length, omega_c);
		}

		///
		/// Generate windowed resonant coefficients (double precision).
		///
		template<>
		inline double* FirFilterUtils::GenerateResonantCoefficients<double>(WindowFunction f, int length, double omega_c) {
			return GenerateCoefficients(f, IdealResonant, length, omega_c);
		}

		///
		/// Generate windowed twin-peak coefficients (double precision).
		///
		template<>
		inline double* FirFilterUtils::GenerateTwinPeakCoefficients<double>(WindowFunction f, int length, double omega_c1, double omega_c2) {
			return GenerateCoefficients(f, IdealTwinPeak, length, omega_c1, omega_c2);
		}

		///
		/// Generate windowed band-pass coefficients (double precision).
		///
		template <>
		inline double* FirFilterUtils::GenerateBandPassCoefficients<double>(WindowFunction f, int length, double omega_c1, double omega_c2) {
			return GenerateCoefficients(f, IdealBandPass, length, omega_c1, omega_c2);
		}

		///
		/// Generate windowed band-stop coefficients (double precision).
		///
		template <>
		inline double* FirFilterUtils::GenerateBandStopCoefficients<double>(WindowFunction f, int length, double omega_c1, double omega_c2) {
			return GenerateCoefficients(f, IdealBandStop, length, omega_c1, omega_c2);
		}


		///
		/// Generate windowed low-pass coefficients (single precision).
		///
		template <>
		inline float* FirFilterUtils::GenerateLowPassCoefficients<float>(WindowFunction f, int length, double omega_c) {
			return fromDouble<float>(GenerateCoefficients(f, IdealLowPass, length, omega_c), length);
		}

		///
		/// Generate windowed high-pass coefficients (single precision).
		///
		template <>
		inline float* FirFilterUtils::GenerateHighPassCoefficients<float>(WindowFunction f, int length, double omega_c) {
			return fromDouble<float>(GenerateCoefficients(f, IdealHighPass, length, omega_c), length);
		}

		///
		/// Generate windowed resonant coefficients (single precision).
		///
		template<>
		inline float* FirFilterUtils::GenerateResonantCoefficients<float>(WindowFunction f, int length, double omega_c) {
			return fromDouble<float>(GenerateCoefficients(f, IdealResonant, length, omega_c), length);
		}

		///
		/// Generate windowed twin-peak coefficients (single precision).
		///
		template<>
		inline float* FirFilterUtils::GenerateTwinPeakCoefficients<float>(WindowFunction f, int length, double omega_c1, double omega_c2) {
			return fromDouble<float>(GenerateCoefficients(f, IdealTwinPeak, length, omega_c1, omega_c2), length);
		}

		///
		/// Generate windowed band-pass coefficients (single precision).
		///
		template <>
		inline float* FirFilterUtils::GenerateBandPassCoefficients<float>(WindowFunction f, int length, double omega_c1, double omega_c2) {
			return fromDouble<float>(GenerateCoefficients(f, IdealBandPass, length, omega_c1, omega_c2), length);
		}

		///
		/// Generate windowed band-stop coefficients (single precision).
		///
		template <>
		inline float* FirFilterUtils::GenerateBandStopCoefficients<float>(WindowFunction f, int length, double omega_c1, double omega_c2) {
			return fromDouble<float>(GenerateCoefficients(f, IdealBandStop, length, omega_c1, omega_c2), length);
		}


		//
		//  Generic FIR filter
		//
		template <typename sample_t, typename coef_t = sample_t>
		class FirFilter : public IFilter<sample_t> {
			private:
				int m_Length;

				coef_t *m_InputPos;
				coef_t *m_History;
				coef_t *m_Coefs;
				coef_t *m_HistEnd;
				coef_t *m_CoefEnd;
				sample_t m_Value;

				// test frequencies for gain calculation
				double corr_f1, corr_f2;

			public:
				// The filter length.
				int GetLength() const { return m_Length; }

				// The overall gain of the coefficients.
				double GetOverallGain();

				// get an array of coefficients
				void GetCoefficients(double *target) const {
					coef_t *cp = m_Coefs;
					coef_t *tp = target;
					for (int i = 0; i != m_Length; ++i) {
						*tp++ = *cp++;
					}
				}

			private:
				// reset the history values to zero
				void Clear() {
					if (m_History) {
						coef_t *h = m_History;
						for (int i = 0; i != m_Length; ++i) {
							*h++ = 0;
						}
					}
					m_InputPos = m_History;
					m_Value = 0;
				}

			public:
				//
				//  ctor for BPF, BSF, and TPF filter types
				//
				FirFilter(FirFilterTypes type, int length, double f1, double f2, size_t fs, WindowFunction wf = HammingWindow);

				//
				//  ctor for LPF, HPF, and resonant filter types
				//
				FirFilter(FirFilterTypes type, int length, double fc, size_t fs, WindowFunction wf = HammingWindow);

				//
				//  ctor for custom coefficients
				//
				FirFilter(int length, const coef_t * coefs);

			public:
				//
				//  run the filter on a new input value, and return the new output value
				//
				sample_t run(sample_t sample) {
					return (m_Value = FirFilterUtils::Filter(sample, m_InputPos, m_History, m_HistEnd, m_Coefs, m_CoefEnd));
				}

				//
				//  return the current output value
				//
				sample_t value() const {
					return m_Value;
				}

				//
				//  run the filter on a new input value
				//
				void add(sample_t sample) {
					run(sample);
				}

				//
				//  clear the history
				//
				void clear() {
					m_Value = 0;
					m_InputPos = m_History;
					while (m_InputPos != m_HistEnd)
						*m_InputPos++ = 0;
					m_InputPos = m_History;
				}
		};


		//
		//  GetOverallGain() - gain calculation
		//
		template <typename sample_t, typename coef_t>
		inline double FirFilter<sample_t, coef_t>::GetOverallGain() {
			// Perform gain correction
			//   It is possible to do a gain calculation directly on the coefficients using
			//   complex exponents, but this will give a real, measured value on the filter itself.
			double freqs[3] = { corr_f1, corr_f2, -1 };
			Clear();
			double gain = FilterUtils::CalculateGain(this, freqs, m_Length);
			Clear();
			return gain;
		}


		//
		//  ctor for BPF, BSF, and TPF filter types
		//
		template <typename sample_t, typename coef_t>
		inline FirFilter<sample_t, coef_t>::FirFilter(FirFilterTypes type, int length, double f1, double f2, size_t fs, WindowFunction wf) {
			// order must be odd
			if ((length % 2) == 0)
				++length;

			// if no window specified, use rectangle
			if (! wf) {
				wf = RectangleWindow;
			}

			m_Length = length;
			double omega_c1 = fs ? (2 * M_PI * f1 / fs) : f1;
			double omega_c2 = fs ? (2 * M_PI * f2 / fs) : f2;

			// generate double coefs, then convert
			switch (type) {
				case BandPass:
					m_Coefs = FirFilterUtils::GenerateBandPassCoefficients<coef_t>(wf, m_Length, omega_c1, omega_c2);
					corr_f1 = 2.0 * M_PI * (f2 + f1) / (2.0 * fs); // measure mid-passband
					break;
				case BandStop:
					m_Coefs = FirFilterUtils::GenerateBandStopCoefficients<coef_t>(wf, m_Length, omega_c1, omega_c2);
					corr_f1 = 2.0 * M_PI * local_min(f2, f1) / (2.0 * fs); // measure mid-passband under the notch
					break;
				case TwinPeak:
					m_Coefs = FirFilterUtils::GenerateTwinPeakCoefficients<coef_t>(wf, m_Length, omega_c1, omega_c2);
					corr_f1 = 2.0 * M_PI * f1 / fs; // measure on each peak
					corr_f2 = 2.0 * M_PI * f2 / fs;
					break;
				case Resonant:
				case LowPass:
				case HighPass:
					throw FirFilterException("This constructor is only for bandpass and bandstop filters");
				default:
					throw FirFilterException("Unknown filter type specified");
			}
			m_History = m_InputPos = new sample_t[m_Length];
			m_HistEnd = m_History + m_Length;
			m_CoefEnd = m_Coefs + m_Length;
		}


		//
		//  ctor for LPF, HPF, and resonant filter types
		//
		template <typename sample_t, typename coef_t>
		inline FirFilter<sample_t, coef_t>::FirFilter(FirFilterTypes type, int length, double fc, size_t fs, WindowFunction wf) {
			// order must be odd
			if ((length % 2) == 0)
				++length;

			// if no window specified, use rectangle
			if (! wf) {
				wf = RectangleWindow;
			}

			m_Length = length;
			double omega_c = fs ? (2 * M_PI * fc / fs) : fc;

			// generate double coefs, then convert
			switch (type) {
				case LowPass:
					m_Coefs = FirFilterUtils::GenerateLowPassCoefficients<coef_t>(wf, m_Length, omega_c);
					corr_f1 = 2.0 * M_PI * fc / (2.0 * fs); // measure gain mid-passband
					break;
				case HighPass:
					m_Coefs = FirFilterUtils::GenerateHighPassCoefficients<coef_t>(wf, m_Length, omega_c);
					corr_f1 = 2.0 * M_PI * (fc + (fs / 2)) / (2.0 * fs); // measure gain mid-passband
					break;
				case Resonant:
					m_Coefs = FirFilterUtils::GenerateResonantCoefficients<coef_t>(wf, m_Length, omega_c);
					corr_f1 = 2.0 * M_PI * fc / fs; // measure gain at the resonant center
					break;
				case TwinPeak:
				case BandPass:
				case BandStop:
					throw FirFilterException("This constructor cannot be used for bandpass, bandstop, and resonant filters");
				default:
					throw FirFilterException("Unknown filter type specified");
			}
			m_History = m_InputPos = new sample_t[m_Length];
			m_HistEnd = m_History + m_Length;
			m_CoefEnd = m_Coefs + m_Length;
		}


		//
		//  ctor for custom coefficients
		//
		template <typename sample_t, typename coef_t>
		inline FirFilter<sample_t, coef_t>::FirFilter(int length, const coef_t * coefs) {
			// order must be odd
			m_Length = length;
			if ((m_Length % 2) == 0)
				++m_Length;
			m_Coefs = new coef_t[m_Length];
			for (size_t i = 0; i != length; ++i) {
				m_Coefs[i] = coefs[i];
			}
			for (size_t i = length; i != m_Length; ++i) {
				m_Coefs = 0;
			}

			m_History = m_InputPos = new sample_t[m_Length];
			m_HistEnd = m_History + m_Length;
			m_CoefEnd = m_Coefs + m_Length;

			m_InputPos = 0;
		}
	}
}
#endif // __KK5JY_FIRFILTER_H
