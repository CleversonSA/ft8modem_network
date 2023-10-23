/*
 *
 *
 *    stype.h
 *
 *    C++ string functions similar to those in ctype.
 *
 *    =========================================================================
 *    
 *    Copyright (C) 2000-2003,2023 by Matt Roberts, KK5JY.
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License as published by the Free Software Foundation; either
 *    version 2.1 of the License, or (at your option) any later version.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *    =========================================================================
 *
 *
 */


#ifndef __STYPE_H
	#define __STYPE_H
	#include <cctype>
	#include <string>
	#include <list>

	namespace my {
		inline std::string ltrim (const std::string &s) {
			std::string::const_iterator si = s.begin();

			// start at begin() and move forward as long as there is whitespace
			while ( (si != s.end()) && (isspace(*si)) ) ++si;

			// copy (s into rtn) from the first non-whitespace character
			//return std::string(s.substr(si - s.begin(), std::string::npos));
			return std::string(si, s.end());
		}

		inline std::string rtrim (const std::string &s) {
			std::string::const_iterator si = s.begin();
			std::string::const_iterator ei = s.begin();

			// step through the string, marking the last non-ws character
			while (si != s.end()) {
				if ( ! isspace(*si++) )
					ei = si;
			}

			// when the last non-ws is found, substr the return value to strip the
			// right-ws
			return s.substr(0, ei - s.begin());
		}

		inline std::string trim (const std::string &s) {
			return my::ltrim(my::rtrim(s));
		}

		inline std::string lstrip (const std::string &s) {
			return my::ltrim(s);
		}

		inline std::string rstrip (const std::string &s) {
			return my::rtrim(s);
		}

		inline std::string strip (const std::string &s) {
			return my::ltrim(my::rtrim(s));
		}

		inline std::string to_something (const std::string &s, int (&func)(int)) {
			std::string::iterator si;
			std::string rtn(s);

			for (si = rtn.begin(); si != rtn.end(); ++si)
				*si = func(*si);

			return std::string(rtn);
		}

		inline bool is_something (const std::string &s, int (&func)(int)) {
			std::string::const_iterator si;
			bool rtn;

			for (rtn = true, si = s.begin(); si != s.end(); ++si)
				rtn &= ((func(*si) == 0) ? false : true);

			return rtn;
		}

		inline std::string toLower (const std::string &s) { return to_something (s, tolower); };
		inline std::string toUpper (const std::string &s) { return to_something (s, toupper); };

		inline bool isAlnum (const std::string &s) { return is_something (s, isalnum); };
		inline bool isAlpha (const std::string &s) { return is_something (s, isalpha); };
		inline bool isAscii (const std::string &s) { return is_something (s, isascii); };
		#ifdef __USE_GNU
		inline bool isBlank (const std::string &s) { return is_something (s, isblank); };
		#endif
		inline bool isControl (const std::string &s) { return is_something (s, iscntrl); };
		inline bool isDigit (const std::string &s) { return is_something (s, isdigit); };
		inline bool isGraph (const std::string &s) { return is_something (s, isgraph); };
		inline bool isLower (const std::string &s) { return is_something (s, islower); };
		inline bool isPrint (const std::string &s) { return is_something (s, isprint); };
		inline bool isPunct (const std::string &s) { return is_something (s, ispunct); };
		inline bool isSpace (const std::string &s) { return is_something (s, isspace); };
		inline bool isUpper (const std::string &s) { return is_something (s, isupper); };
		inline bool isXDigit (const std::string &s) { return is_something (s, isxdigit); };

		std::list<std::string> split (const std::string &s);
		std::list<std::string> split (const std::string &s, std::string::value_type delim, bool strip = false);
	}

#endif /* _STYPE_H */

