/*
 *
 *
 *    nlimits.cpp
 *
 *    Normalized numeric limits.
 *
 *    Copyright (C) 2016 by Matt Roberts.
 *    License: GNU GPL3 (www.gnu.org)
 *
 *
 */

#include <stdint.h>
#include "nlimits.h"

namespace KK5JY {
	namespace DSP {
		// int8
		template<>
		const int8_t norm_limits<int8_t>::maximum = 127;

		template<>
		const int8_t norm_limits<int8_t>::minimum = -128;

		template<>
		const double norm_limits<int8_t>::range = 256;

		template<>
		const bool norm_limits<int8_t>::is_signed = true;

		// uint8
		template<>
		const uint8_t norm_limits<uint8_t>::maximum = 255;

		template<>
		const uint8_t norm_limits<uint8_t>::minimum = 0;

		template<>
		const double norm_limits<uint8_t>::range = 256;

		template<>
		const bool norm_limits<uint8_t>::is_signed = false;

		// int16
		template<>
		const int16_t norm_limits<int16_t>::maximum = 32767;

		template<>
		const int16_t norm_limits<int16_t>::minimum = -32768;

		template<>
		const double norm_limits<int16_t>::range = 65536;

		template<>
		const bool norm_limits<int16_t>::is_signed = true;

		// uint16
		template<>
		const uint16_t norm_limits<uint16_t>::maximum = 65535;

		template<>
		const uint16_t norm_limits<uint16_t>::minimum = 0;

		template<>
		const double norm_limits<uint16_t>::range = 65536;

		template<>
		const bool norm_limits<uint16_t>::is_signed = false;

		// int32
		template<>
		const int32_t norm_limits<int32_t>::maximum = 2147483647L;

		template<>
		const int32_t norm_limits<int32_t>::minimum = -2147483648L;

		template<>
		const double norm_limits<int32_t>::range = 4294967295UL;

		template<>
		const bool norm_limits<int32_t>::is_signed = true;

		// uint32
		template<>
		const uint32_t norm_limits<uint32_t>::maximum = 4294967295UL;

		template<>
		const uint32_t norm_limits<uint32_t>::minimum = 0;

		template<>
		const double norm_limits<uint32_t>::range = 4294967295UL;

		template<>
		const bool norm_limits<uint32_t>::is_signed = false;

		//
		//  floating limits (-1, 1)
		//
		template<>
		const float norm_limits<float>::maximum = 1.0;

		template<>
		const float norm_limits<float>::minimum = -1.0;

		template<>
		const double norm_limits<float>::range = 2.0;

		template<>
		const double norm_limits<double>::maximum = 1.0;

		template<>
		const double norm_limits<double>::minimum = -1.0;

		template<>
		const double norm_limits<double>::range = 2.0;
	}
}

// EOF
