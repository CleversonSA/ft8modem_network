/*
 *
 *
 *    shape.h
 *
 *    Raised cosine envelope shaping.
 *
 *    Copyright (C) 2023 by Matt Roberts.
 *    License: GNU GPL3 (www.gnu.org)
 *
 *
 */

#ifndef __KK5JY_SHAPE_H
#define __KK5JY_SHAPE_H

#include <math.h>
#include <stdint.h>
#include "nlimits.h"
#include "IFilter.h"

namespace KK5JY {
	namespace DSP {
		// ---------------------------------------------------------------------------
		//
		//   class Shaper<T>
		//
		//   Raised-cosine shaping.
		//
		// ---------------------------------------------------------------------------
		template <typename T, typename ctr_t = uint16_t>
		class Shaper {
			private:
				const ctr_t samples;
				const T phi;
				ctr_t ctr;
			public:
				Shaper(ctr_t _samp);
				T run(bool key);
				ctr_t size() const { return samples; }
		};

		//
		//  Shaper::ctor
		//
		template <typename T, typename ctr_t>
		Shaper<T, ctr_t>::Shaper(ctr_t _samp)
			: samples(_samp), phi(M_PI / samples), ctr(0)
		{
			// nop
		}

		//
		//  Shaper::run
		//
		template <typename T, typename ctr_t>
		T Shaper<T, ctr_t>::run(bool key) {
			if (key) {
				if (ctr < samples) {
					++ctr;
				} else {
					return norm_limits<T>::maximum;
				}
			} else {
				if (ctr > 0) {
					--ctr;
				} else {
					return 0;
				}
			}

			return norm_limits<T>::maximum * (0.5 + (-0.5 * ::cos(ctr * phi)));
		}
	}
}

#endif // __KK5JY_SMOOTH_H
