/*
 *
 *
 *    nlimits.h
 *
 *    Normalized numeric limits.
 *
 *    Copyright (C) 2016 by Matt Roberts.
 *    License: GNU GPL3 (www.gnu.org)
 *
 *
 */

#ifndef __KK5JY_NLIMITS_H
#define __KK5JY_NLIMITS_H

namespace KK5JY {
	namespace DSP {
		// normalization limits
		template<typename sample_t>
		struct norm_limits {
			const static sample_t maximum;
			const static sample_t minimum;
			const static double range;
			const static bool is_signed;
		};
	}
}

#endif
