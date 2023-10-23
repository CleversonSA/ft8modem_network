/*
 *
 *
 *    WindowFunctions.h
 *
 *    Window functions.
 *
 *    Copyright (C) 2013-2021 by Matt Roberts, KK5JY.
 *    All rights reserved.
 *
 *    License: GNU GPL3 (www.gnu.org)
 *    
 *
 */

#ifndef KK5JY_WINDOWFUNCTIONS_H
#define KK5JY_WINDOWFUNCTIONS_H

#include <cmath>

namespace KK5JY {
	namespace DSP {
		/// <summary>
		/// Generate Hamming coefficiencts for an odd-length window centered at zero, of length N.
		/// </summary>
		/// <param name="n">The sample number.</param>
		/// <param name="N">The window length.</param>
		inline double HammingWindow(int n, int N) {
			return 0.54 - (0.46 * cos((2.0 * M_PI * (n + (N / 2))) / (N - 1)));
		}

		/// <summary>
		/// Generate Hann coefficiencts for an odd-length window centered at zero, of length N.
		/// </summary>
		/// <param name="n">The sample number.</param>
		/// <param name="N">The window length.</param>
		inline double HannWindow(int n, int N) {
			return 0.50 * (1.0 - cos((2.0 * M_PI * (n + (N / 2))) / (N - 1)));
		}

		/// <summary>
		/// Generate "exact" Blackman coefficiencts for an odd-length window centered at zero, of length N.
		/// </summary>
		/// <param name="n">The sample number.</param>
		/// <param name="N">The window length.</param>
		inline double BlackmanExactWindow(int n, int N) {
			return (7938.0 / 18608.0)
				- ((9240.0 / 18608.0) * cos((2.0 * M_PI * (n + (N / 2))) / (N - 1)))
				+ ((1430.0 / 18608.0) * cos((4.0 * M_PI * (n + (N / 2))) / (N - 1)));
		}

		/// <summary>
		/// Generate Blackman coefficiencts for an odd-length window centered at zero, of length N.
		/// </summary>
		/// <param name="n">The sample number.</param>
		/// <param name="N">The window length.</param>
		inline double BlackmanWindow(int n, int N) {
			return 0.426590
				- (0.496560 * cos((2.0 * M_PI * (n + (N / 2))) / (N - 1)))
				+ (0.076849 * cos((4.0 * M_PI * (n + (N / 2))) / (N - 1)));
		}

		/// <summary>
		/// Generate Nuttall coefficiencts for an odd-length window centered at zero, of length N.
		/// </summary>
		/// <param name="n">The sample number.</param>
		/// <param name="N">The window length.</param>
		inline double NuttallWindow(int n, int N) {
			return 0.355768
				- (0.487396 * cos((2.0 * M_PI * (n + (N / 2))) / (N - 1)))
				+ (0.144232 * cos((4.0 * M_PI * (n + (N / 2))) / (N - 1)))
				- (0.012604 * cos((6.0 * M_PI * (n + (N / 2))) / (N - 1)));
		}

		/// <summary>
		/// Generate Blackman-Nuttall coefficiencts for an odd-length window centered at zero, of length N.
		/// </summary>
		/// <param name="n">The sample number.</param>
		/// <param name="N">The window length.</param>
		inline double BlackmanNuttallWindow(int n, int N) {
			return 0.3635819
				- (0.4891775 * cos((2.0 * M_PI * (n + (N / 2))) / (N - 1)))
				+ (0.1365995 * cos((4.0 * M_PI * (n + (N / 2))) / (N - 1)))
				- (0.0106511 * cos((6.0 * M_PI * (n + (N / 2))) / (N - 1)));
		}

		/// <summary>
		/// Generate Blackman-Harris coefficiencts for an odd-length window centered at zero, of length N.
		/// </summary>
		/// <param name="n">The sample number.</param>
		/// <param name="N">The window length.</param>
		inline double BlackmanHarrisWindow(int n, int N) {
			return 0.35875
				- (0.48829 * cos((2.0 * M_PI * (n + (N / 2))) / (N - 1)))
				+ (0.14128 * cos((4.0 * M_PI * (n + (N / 2))) / (N - 1)))
				- (0.01168 * cos((6.0 * M_PI * (n + (N / 2))) / (N - 1)));
		}

		/// <summary>
		/// Generate flat-top coefficiencts for an odd-length window centered at zero, of length N.
		/// </summary>
		/// <param name="n">The sample number.</param>
		/// <param name="N">The window length.</param>
		inline double FlatTopWindow(int n, int N) {
			return 1.0
				- (1.93 * cos((2.0 * M_PI * (n + (N / 2))) / (N - 1)))
				+ (1.29 * cos((4.0 * M_PI * (n + (N / 2))) / (N - 1)))
				- (0.388 * cos((6.0 * M_PI * (n + (N / 2))) / (N - 1)))
				+ (0.028 * cos((6.0 * M_PI * (n + (N / 2))) / (N - 1)));
		}

		/// <summary>
		/// Generate Rectangle coefficiencts for an odd-length window centered at zero, of length N.
		/// </summary>
		/// <param name="n">The sample number.</param>
		/// <param name="N">The window length.</param>
		inline double RectangleWindow(int n, int N) {
			return 1.0;
		}
	}
}

#endif // KK5JY_WINDOWFUNCTIONS_H
