/*
 *
 *
 *    IFilter.h
 *
 *    Filter interface.
 *
 *    Copyright (C) 2016-2021 by Matt Roberts,
 *    All rights reserved.
 *
 *    License: GNU GPL3 (www.gnu.org)
 *
 *
 */

#ifndef __KK5JY_IFILTER_H
#define __KK5JY_IFILTER_H

template <typename sample_t>
class IFilter {
	public:
		// run the filter and return the updated value of the filter
		virtual sample_t run(const sample_t sample) = 0;

		// update the the filter, but don't return a new output value;
		//    this is helpful if calculating the return value is costly
		virtual void add(const sample_t sample) = 0;

		// return the current value of the filter without changing it
		virtual sample_t value() const = 0;

		// erase the history in the filter
		virtual void clear() = 0;

		// virtual dtor
		virtual ~IFilter() { /* nop */ };
};

#endif // __KK5JY_IFILTER_H
