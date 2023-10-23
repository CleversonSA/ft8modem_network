/*
 *
 *
 *    es.h
 *
 *    Exponential smoothers.
 *
 *    Copyright (C) 2017-2021 by Matt Roberts.
 *    License: GNU GPL3 (www.gnu.org)
 *
 *
 */

#ifndef __KK5JY_ES_H
#define __KK5JY_ES_H

#include <math.h>
#include <stdint.h>
#include "nlimits.h"
#include "IFilter.h"

namespace KK5JY {
	namespace DSP {
		//
		//  Calculate alpha value to emulate RC lowpass filter in smoother.
		//
		inline double HighpassToAlpha(size_t fs, double fc) {
			// caluclate the equivalent R-C constant
			double rc = 1.0 / (2.0 * M_PI * fc);

			// calculate the sampling period
			double dt = 1.0 / fs;

			// calculate equivalent alpha value
			return rc / (rc + dt);
		}

		//
		//  calculate lowpass fc value from alpha when using smoother as LP filter.
		//
		inline double AlphaToHighpass(double alpha, size_t fs) {
			// calculate the sampling period
			double dt = 1.0 / fs;

			// calculate RC value from alpha
			double rc = (dt * (1.0 - alpha)) / alpha;

			// caluclate fc from RC
			return 1.0 / (2.0 * M_PI * rc);
		}

		//
		//  Calculate alpha value to emulate RC lowpass filter in smoother.
		//
		inline double LowpassToAlpha(size_t fs, double fc) {
			// caluclate the equivalent R-C constant
			double rc = 1.0 / (2.0 * M_PI * fc);

			// calculate the sampling period
			double dt = 1.0 / fs;

			// calculate equivalent alpha value
			return dt / (rc + dt);
		}

		//
		//  calculate lowpass fc value from alpha when using smoother as LP filter.
		//
		inline double AlphaToLowpass(double alpha, size_t fs) {
			// calculate the sampling period
			double dt = 1.0 / fs;

			// calculate RC value from alpha
			double rc = (dt * (1.0 - alpha)) / alpha;

			// caluclate fc from RC
			return 1.0 / (2.0 * M_PI * rc);
		}


		// ---------------------------------------------------------------------------
		//
		//   class Smoother<T>
		//
		//   Exponential smoothing class for floating point types.
		//   This is also an approximation of a first-order RC lowpass circuit.
		//
		// ---------------------------------------------------------------------------
		template <typename T>
		class Smoother : public IFilter<T> {
			public:
				const T Alpha;
			private:
				T state;
			public:
				Smoother(double alpha, T seed = 0);
				T run(T in);
				void clear(void);
				T value() const { return state; }
				void add(T s) { run(s); }
		};

		//
		//  Smoother::ctor
		//
		template <typename T>
		Smoother<T>::Smoother(double alpha, T seed)
			: Alpha(alpha), state(seed)
		{
			// nop
		}

		//
		//  Smoother::ctor - int16 specialization
		//
		template <>
		inline Smoother<int16_t>::Smoother(double alpha, int16_t seed)
			: Alpha(alpha * norm_limits<int16_t>::maximum), state(seed)
		{
			// nop
		}

		//
		//  Smoother::ctor - int32 specialization
		//
		template <>
		inline Smoother<int32_t>::Smoother(double alpha, int32_t seed)
			: Alpha(alpha * norm_limits<int32_t>::maximum), state(seed)
		{
			// nop
		}

		//
		//  Smoother::run
		//
		template <typename T>
		T Smoother<T>::run(T in) {
			return state = (in * Alpha) + (state * (1.0 - Alpha));
		}

		//
		//  Smoother::run - int16 specialization
		//
		template <>
		inline int16_t Smoother<int16_t>::run(int16_t in) {
			return state =
				((in * Alpha) >> 15) +
				((state * (norm_limits<int16_t>::maximum - Alpha)) >> 15);
		}

		//
		//  Smoother::run - int32 specialization
		//
		template <>
		inline int32_t Smoother<int32_t>::run(int32_t in) {
			return state =
				((static_cast<int64_t>(in) * static_cast<int64_t>(Alpha)) >> 31) +
				((static_cast<int64_t>(state) * (norm_limits<int32_t>::maximum - Alpha)) >> 31);
		}

		//
		//  Smoother::clear
		//
		template <typename T>
		void Smoother<T>::clear() {
			state = 0;
		}


		// ---------------------------------------------------------------------------
		//
		//   class Desmoother<T>
		//
		//   First-order approximation of a high-pass RC filter
		//
		// ---------------------------------------------------------------------------
		template <typename T>
		class Desmoother : public IFilter<T> {
			public:
				const T Alpha;
			private:
				T x1, y1;
			public:
				Desmoother(double alpha, T seed = 0);
				T run(T in);
				void clear();
				void add(T s) { run(s); }
				T value() const { return y1; }
		};

		//
		//  Desmoother::ctor
		//
		template <typename T>
		Desmoother<T>::Desmoother(double alpha, T seed)
			: Alpha(alpha), x1(seed), y1(seed)
		{
			// nop
		}

		//
		//  Desmoother::ctor - int16 specialization
		//
		template <>
		inline Desmoother<int16_t>::Desmoother(double alpha, int16_t seed)
			: Alpha(alpha * norm_limits<int16_t>::maximum), x1(seed), y1(seed)
		{
			// nop
		}

		//
		//  Desmoother::ctor - int32 specialization
		//
		template <>
		inline Desmoother<int32_t>::Desmoother(double alpha, int32_t seed)
			: Alpha(alpha * norm_limits<int32_t>::maximum), x1(seed), y1(seed)
		{
			// nop
		}

		//
		//  Desmoother::run
		//
		template <typename T>
		T Desmoother<T>::run(T x0) {
			y1 = (Alpha * y1) + (Alpha * (x0 - x1));
			x1 = x0;
			return y1;
		}

		//
		//  Desmoother::run - int16 specialization
		//
		template <>
		inline int16_t Desmoother<int16_t>::run(int16_t x0) {
			y1 = ((Alpha * y1) >> 15) + ((Alpha * (x0 - x1)) >> 15);
			x1 = x0;
			return y1;
		}

		//
		//  Desmoother::run - int32 specialization
		//
		template <>
		inline int32_t Desmoother<int32_t>::run(int32_t x0) {
			y1 =
				((static_cast<int64_t>(Alpha) * static_cast<int64_t>(y1)) >> 31) +
				((static_cast<int64_t>(Alpha) * static_cast<int64_t>(x0 - x1)) >> 31);
			x1 = x0;
			return y1;
		}

		//
		//  Desmoother::clera
		//
		template <typename T>
		void Desmoother<T>::clear() {
			x1 = y1 = 0;
		}

		// ---------------------------------------------------------------------------
		//
		//   class Decay<T>
		//
		//   Same as Smoother<T>, but with fast-rise; approximates an RC circuit
		//   with a diode pointing in the charging direction.
		//
		// ---------------------------------------------------------------------------
		template <typename T>
		class Decay : public IFilter<T> {
			public:
				const T Alpha;
			private:
				T state;
			public:
				Decay(double alpha, T seed = 0);

				T run(T in);
				T value() const { return state; }
				void add(T in) { run(in); }
				void clear();
		};

		//
		//  Decay::ctor
		//
		template <typename T>
		inline Decay<T>::Decay(double alpha, T seed)
			: Alpha(alpha), state(seed) {
			// nop
		}

		//
		//  Decay::ctor - int16 specialization
		//
		template <>
		inline Decay<int16_t>::Decay(double alpha, int16_t seed)
			: Alpha(norm_limits<int16_t>::maximum * alpha), state(seed) {
			// nop
		}

		//
		//  Decay::ctor - int32 specialization
		//
		template <>
		inline Decay<int32_t>::Decay(double alpha, int32_t seed)
			: Alpha(norm_limits<int32_t>::maximum * alpha), state(seed) {
			// nop
		}

		//
		//  Decay::run
		//
		template <typename T>
		inline T Decay<T>::run(T in) {
			if (in >= state) return state = in;
			return state = (in * Alpha) + (state * (1.0 - Alpha));
		}

		//
		//  Decay::run - int16 specialization
		//
		template <>
		inline int16_t Decay<int16_t>::run(int16_t in) {
			if (in >= state) return state = in;
			return state = ((in * Alpha) >> 15) + ((state * (norm_limits<int16_t>::maximum - Alpha)) >> 15);
		}

		//
		//  Decay::run - int32 specialization
		//
		template <>
		inline int32_t Decay<int32_t>::run(int32_t in) {
			if (in >= state) return state = in;
			return
				state = ((static_cast<int64_t>(in) * Alpha) >> 31) +
				((static_cast<int64_t>(state) * (norm_limits<int32_t>::maximum - Alpha)) >> 31);
		}

		//
		//  Decay::clear
		//
		template <typename T>
		inline void Decay<T>::clear() {
			state = 0;
		}
	}
}

#endif // __KK5JY_ES_H
