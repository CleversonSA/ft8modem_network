/*
 *
 *   sc.h - soundcard interface
 *
 *   This is a class library that wraps an RtAudio soundcard object
 *   and simplifies the interface.
 *
 *   Copyright (C) 2015-2018 by Matt Roberts, KK5JY,
 *   All rights reserved.
 *
 *   License: GNU GPL3 (www.gnu.org)
 *
 */

#ifndef __KK5JY_SC_H
#define __KK5JY_SC_H

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <string>
#include <rtaudio/RtAudio.h>


//
//  SoundCard - simple mono full-duplex interface to the sound card
//
class SoundCard {
	public:
		RtAudio adc;
		RtAudio::StreamParameters params;
		unsigned mCard;
		unsigned mRate;
		unsigned mChannels;
		unsigned mWin;
	
	public:
		SoundCard(unsigned id, unsigned rate, unsigned short channels = 2, unsigned short win = 256);
		virtual ~SoundCard() { };

	public:
		virtual bool start();
		virtual void stop();

	public:
		static void showDevices();
		static unsigned deviceCount();
	
	protected:
		virtual void event(float *inBuffer, float *outBuffer, size_t samples) = 0;

	private:
		static int handler(
			void *outputBuffer,
			void *inputBuffer,
			unsigned int nBufferFrames,
			double streamTime,
			RtAudioStreamStatus status,
			void *userData );
};

/*
 *
 *  SoundCard::ctor(...)
 *
 */
inline SoundCard::SoundCard(unsigned id, unsigned rate, unsigned short channels, unsigned short win)
	: adc(RtAudio::LINUX_ALSA),
	  mCard(id),
	  mRate(rate),
	  mChannels(channels),
	  mWin(win) {
	// nop
}

/*
 *
 *  SoundCard::start()
 *
 */
inline bool SoundCard::start() {
	params.deviceId = mCard;
	params.nChannels = mChannels;
	params.firstChannel = 0;

	// open the sound card
	try {
		adc.openStream(
			&params, // output
			&params, // input
			RTAUDIO_FLOAT32, // format
			mRate,  // rate
			&mWin, // buffer size
			&handler,     // callback
			this);        // callback user data
		// start
		adc.startStream();
	}
	catch ( RtAudioError& e ) {
		return false;
	}
	return true;
}

/*
 *
 *  SoundCard::stop()
 *
 */
inline void SoundCard::stop() {
	if ( adc.isStreamOpen() )
		adc.stopStream();
}

/*
 *
 *   SoundCard::handler(...)
 *
 */
inline int SoundCard::handler(
		void *outputBuffer,
		void *inputBuffer,
		unsigned int nBufferFrames,
		double streamTime,
		RtAudioStreamStatus status,
		void *sc) {
	#ifdef _DEBUG
	if (status) {
		std::cerr << "[sc:ov]";
	}
	#endif

	// extract appropriate pointers
	SoundCard *thisPtr = (SoundCard*)(sc);
	if (thisPtr == 0) return 0;
	float *inData = (float*)(inputBuffer);
	if (inData == 0) return 0;
	float *outData = (float*)(outputBuffer);
	if (outData == 0) return 0;

	// call the user's handler
	thisPtr->event(inData, outData, nBufferFrames);

	// return success
	return 0;
}


/*
 *
 *   channelsToString(...)
 *
 */
static std::string channelsToString(unsigned count) {
	switch(count) {
		case 0: return "None";
		case 1: return "Mono";
		case 2: return "Stereo";
		default: return "Multi";
	}
}

/*
 *
 *   ratesToString(...)
 *
 */
static std::string ratesToString(std::vector<unsigned> rates) {
	std::stringstream result;
	for (unsigned i = 0; i != rates.size(); ++i) {
		if (result.str().size() != 0)
			result << ", ";
		result << rates[i];
	}
	return result.str();
}

/*
 *
 *   showDevices()
 *
 */
inline void SoundCard::showDevices (void) {
	RtAudio adc(RtAudio::LINUX_ALSA);

	// Determine the number of devices available
	unsigned int devices = adc.getDeviceCount();
	if (devices == 0) {
		std::cout << "No audio devices found." << std::endl;
		return;
	}

	// Scan through devices for various capabilities
	RtAudio::DeviceInfo info;
	std::cout << "Valid devices:" << std::endl;
	for (unsigned int i = 0; i < devices; i++) {
		info = adc.getDeviceInfo(i);
		if (info.probed == true) {
			// Print, for example, the maximum number of output channels for each device
			std::cout << " + Device ID = " << i;
			std::cout << ": \"" << info.name << "\"";
			std::cout << ", inputs = " << channelsToString(info.inputChannels);
			std::cout << ", outputs = " << channelsToString(info.outputChannels);
			std::cout << ", rates = " << ratesToString(info.sampleRates);
			std::cout << "\n";
		}
	}
}

/*
 *
 *   deviceCount()
 *
 */
inline unsigned SoundCard::deviceCount() {
	RtAudio adc(RtAudio::LINUX_ALSA);
	return adc.getDeviceCount();
}

#endif // __KK5JY_SC_H
