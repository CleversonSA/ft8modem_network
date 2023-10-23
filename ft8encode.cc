/*
 *
 *
 *    ft8encode.cc
 *
 *    WAV encoder for FT8.
 *
 *    Copyright (C) 2023 by Matt Roberts.
 *    License: GNU GPL3 (www.gnu.org)
 *
 *
 */

#include <iostream>
#include <stdexcept>
#include <unistd.h>
#include "encode.h"
#include "mfsk.h"
#include "sf.h"
#include "stype.h"

using namespace KK5JY::DSP::MFSK;
using namespace std;

int main(int argc, char**argv) {
	if (argc != 6) {
		cerr << "Usage: " << argv[0] << " <mode> <fs> <freq> <wav> '<txt>'" << endl;
		return 1;
	}

	// extract arguments
	std::string mode = argv[1];
	std::string wav = argv[4];
	std::string txt = argv[5];
	double rate = atof(argv[2]);
	double f0 = atof(argv[3]);
	double bps, shift;

	std::string rmode = my::toLower(mode);
	if (mode == "ft8") {
		bps = 6.25;
		shift = bps;
	} else if (mode == "ft4") {
		bps = 12000.0 / 576.0; //23.391812865497077; 
		shift = bps;
	} else {
		cerr << "Invalid mode." << std::endl;
		return 1;
	}

	// open a new WAV file
	SoundFile output(wav, rate, 1, SoundFile::major_formats::wav, SoundFile::minor_formats::s16);

	// DEBUG:
	//cerr << "Using rate = " << rate << "; bps = " << bps << "; shift = " << shift << "; txt = " << txt << endl;

	// create a new modulator
	KK5JY::DSP::MFSK::Modulator<float> mfsk(rate, f0, bps, shift);
	mfsk.setVolume(0.5);

	// encode
	mfsk.transmit(KK5JY::FT8::encode(mode, txt), f0);

	// write
	size_t count = 0;
	size_t samples = 0;
	float buffer[128];
	const size_t buflen = sizeof(buffer) / sizeof(buffer[0]);
	do {
		count = mfsk.read(buffer, buflen);
		size_t ct2 = output.write(buffer, count);
		if (count != ct2)
			throw runtime_error("Output mismatch");

		samples += count;
	} while (count != 0);
	float *ep = buffer + buflen;
	float *zp = buffer;
	while (zp != ep) {
		*zp++ = 0;
	}
	size_t target = samples + (rate / 2);
	do {
		samples += output.write(buffer, buflen);
	} while (samples < target);

	// DEBUG:
	cerr << "Wrote " << samples << " samples (" << static_cast<double>(samples) / rate << " sec)." << endl;

	// done
	return 0;
}

// EOF
