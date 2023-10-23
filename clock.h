/*
 *
 *
 *    clock.h
 *
 *    Wall clock helper methods.
 *
 *    Copyright (C) 2023 by Matt Roberts.
 *    License: GNU GPL3 (www.gnu.org)
 *
 *
 */

#ifndef __KK5JY_FT8_CLOCK_H
#define __KK5JY_FT8_CLOCK_H

#include <sys/time.h>
#include <math.h>

namespace KK5JY {
	namespace FT8 {
		//
		//  abstime() - return absolute clock in seconds
		//
		double abstime() {
			// read the clock
			struct timeval tv;
			struct timezone tz;
			::gettimeofday(&tv, &tz);

			double result = static_cast<double>(tv.tv_sec);
			result += static_cast<double>(tv.tv_usec) / 1000000.0;
			return result;
		}


		//
		//  class FrameClock
		//
		class FrameClock {
			private:
				// a known 'zero' seconds on the clock
				time_t m_epoch;
				
			public:
				// ctor
				FrameClock();

				// return the number of seconds in the current minute
				//    with microsecond precision
				double seconds(double mod = 60) const volatile;
		};


		//
		//  FrameClock() ctor
		//
		inline FrameClock::FrameClock() {
			// read the clock
			struct timeval tv;
			struct timezone tz;
			::gettimeofday(&tv, &tz);

			// now convert that into HMS
			time_t time = tv.tv_sec;
			tm *hms = gmtime(&time);
			
			// figure out where 'zero' is relative to 'tv.tv_sec'
			tv.tv_sec -= hms->tm_sec;
			m_epoch = tv.tv_sec;
		}


		//
		//  FrameClock::seconds()
		//
		inline double FrameClock::seconds(double mod) const volatile {
			// read the clock
			struct timeval tv;
			struct timezone tz;
			::gettimeofday(&tv, &tz);

			double result = fmod(static_cast<double>(tv.tv_sec - m_epoch), mod);
			result += static_cast<double>(tv.tv_usec) / 1000000.0;
			return result;
		}
	}
}

#endif // __KK5JY_FT8_CLOCK_H
