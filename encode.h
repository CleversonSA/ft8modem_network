/*
 *
 *
 *    encode.h
 *
 *    Conversion from text to FT8 or FT4 keying symbols.
 *
 *    Copyright (C) 2023 by Matt Roberts.
 *    License: GNU GPL3 (www.gnu.org)
 *
 *
 */

#include <stdio.h>
#include <string>
#include <stdexcept>
#include "stype.h"

namespace KK5JY {
	namespace FT8 {
		//
		//  return the keying symbols for a message
		//
		inline std::string encode(const std::string &mode, const std::string &txt) {
			std::string tool;
			std::string rmode = my::toLower(mode);
			if (rmode == "ft8") {
				tool = "ft8code";
			} else if (rmode == "ft4") {
				tool = "ft4code";
			} else {
				throw std::runtime_error("Invalid mode provided");
			}

			// use 'ft[48]code' to generate the message symbols
			std::string cmd = tool + " \"" + txt + "\" | tail -1";
			FILE *code = popen(cmd.c_str(), "r");

			#ifdef VERBOSE_DEBUG
			std::cerr << "popen(" << cmd << ")" << std::endl;
			#endif

			if ( ! code) {
				#ifdef VERBOSE_DEBUG
				std::cerr << "ft8code failed" << std::endl;
				#endif

				// return failure
				return "";
			}

			// I/O loop on 'ft8code' output
			char iobuffer[128];
			std::string linebuffer;
			while ( ! feof(code)) {
				size_t ct = fread(iobuffer, 1, sizeof(iobuffer), code);
				linebuffer.append(iobuffer, ct);
				if (ct == 0)
					break;
			}

			// close the pipe to the child
			fclose(code);

			// trim the string
			linebuffer = my::strip(linebuffer);

			#ifdef VERBOSE_DEBUG
			std::cerr << "ft8code returned: " << linebuffer << std::endl;
			#endif

			return linebuffer;
		}
	}
}

// EOF
