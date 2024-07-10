/*
    Software modem for FT8/FT4.
    Copyright (C) 2023  Matt Roberts (Creator/Original project)
				  2023	Cleverson S A (Network and aditional features support)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#define MAX_DECODED_MESSAGES 16

#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdio>
using std::sprintf;
using std::string;
using std::strlen;
using std::ofstream;
using std::strtok;
using std::strstr;
using std::strcmp;

#include <vector>
using std::vector;

#include "snddev.h"

//
// Call Sign database
//
#include "call_sign_driver.h"
CallSignCountryDriver hamOperatorCountry;

//
// TCP Socket
//
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#define PORT 6666
int server_fd, new_socket;


using namespace std;
using namespace KK5JY::DSP;
using namespace KK5JY::DSP::MFSK;

//
// Prototypes
//
void usage(const std::string &s);
void handleDecodedMessages(vector<DecodedLine> * newMessagesPtr);
void printDecodedMessages();
void wipeDecodedMessages();
void *asyncDecodeMessage(void * arg);
void interpretCommand(string *, ModemSoundDevice* audio);
void printCallSignCountry(char *);


//
// Main cache for decoded messages
//
vector <DecodedLine> cacheDecodedMessages;
int decodedMessageQt = 0;
bool cqOnlyEnabled = false;


//
//  main()
//
int main(int argc, char**argv) {

	// Network
	struct sockaddr_in address;
	int opt = 1;
	int addrlen = sizeof(address);

	// async decoding
	pthread_t asyncDecodeThreads[1];


	// basic validation
	if (argc < 3) {
		usage(argv[0]);
		return 1;
	}

	// read arguments
	std::string mode = argv[1];
	int dev = atoi(argv[2]);
	short depth = 2; // 2 = Normal
	if (argc == 3)
		depth = atoi(argv[2]);
	
	cout << "Selected card is " << dev << endl;
	// initialize sound card
	ModemSoundDevice audio(mode, dev, 48000, 256);
	audio.setDepth(depth);
	audio.setVolume(0.5);
	audio.start();

	// Start decoding thread
	int ret = pthread_create(&asyncDecodeThreads[0], NULL, asyncDecodeMessage, (void *)(&audio));
	if(ret != 0) {
		printf("Error: pthread_create() failed\n");
		exit(EXIT_FAILURE);
	}
	//pthread_join(asyncDecodeThreads[0], (void **)&ret);
	cout << "App Initialized" << endl;

	// read transmit messages
	std::string msg = "";
	char iobuffer[16];
	bool active = false;


	// Creating socket file descriptor
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket failed");
		exit(EXIT_FAILURE);
	}


	// Forcefully attach socket to the port 6666
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) 
	{
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);



	// Network start
   	 if (bind(server_fd, (struct sockaddr*)&address,
        	     sizeof(address)) < 0) {
        	perror("bind failed");
        	exit(EXIT_FAILURE);
    	}
    	if (listen(server_fd, 3) < 0) {
        	perror("listen");
        	exit(EXIT_FAILURE);
    	}

	while (true) {

		if ((new_socket
			= accept(server_fd, (struct sockaddr*)&address,
					(socklen_t*)&addrlen))
			< 0) {
			perror("accept");
			exit(EXIT_FAILURE);
		}

		int ct = 0;
		
		while (true) {

			if (!active) {
				active = audio.isActive();
				if (active)
					cout << "INFO: Sound callback is active." << endl;
			}

			//read from network
			ct = read(new_socket, iobuffer, sizeof(iobuffer));

			// if EOF, close the program
			if (ct <= 0) {
				cout << "None" << endl;
				break;
			}

			// process data from stdin
			for (int i = 0; i != ct; ++i) {
				char ch = iobuffer[i];

				// drop non-digits
				if (isalnum(ch) 
					|| ch == ' ' 
					|| ch == '.' 
					|| ch == '-' 
					|| ch == '+'
					|| ch == ';') {
					msg += ch;
				}

				// terminate line
				if (ch == '\n' || ch == '\r') {
					
					cout << "Command Received:\"" << msg << "\"" << endl;

					interpretCommand(&msg, &audio);
					msg.clear();
					break;

				}
			}
		}

		// closing the connected socket
		close(new_socket);

	}

	// stop the sound card
	audio.stop();

    // closing the listening socket
    shutdown(server_fd, SHUT_RDWR);

	
	// done
	return 0;
}


//
// Functions
// =====================================================================
//

//
//  Try identify the country of a call sign
//
void printCallSignCountry(string &callSign)
{
	char countryAssinged[100];
	countryAssinged[0] = '\0';

	sprintf(countryAssinged, "QRZCOUNTRY;%s\n\r", hamOperatorCountry.getCountry(callSign).c_str());
	send(new_socket, countryAssinged, strlen(countryAssinged), 0);
	
}

//
//  Interpret the command in StdIn or Socket or Serial
//
void interpretCommand(string * msg, ModemSoundDevice* audio)
{

	if (my::toUpper((*msg)) == "CQONLYENABLED") {
		(*msg).clear();
		cqOnlyEnabled = true;
		wipeDecodedMessages();
		cout << "Only CQ requests will be listed!" << endl;
		return;
	}

	if (my::toUpper((*msg)) == "CQONLYDISABLED") {
		(*msg).clear();
		cqOnlyEnabled = false;
		wipeDecodedMessages();
		cout << "All band activity will be listed!" << endl;
		return;
	}

	if (my::toUpper((*msg)) == "WIPE") {
		(*msg).clear();
		wipeDecodedMessages();
		return;
	}

	if (my::toUpper((*msg)) == "LOGS") {
		(*msg).clear();
		printDecodedMessages();
		return;
	}

	if (my::toUpper((*msg)) == "STOP") {
		cout << "INFO: Cancel transmit" << endl;
		(*audio).cancelTransmit();
		(*msg).clear();
		return;
	}

	size_t idx = (*msg).find("QRZCOUNTRY");
	if (idx != std::string::npos) {
		
		idx = (*msg).find(';');
		if (idx == std::string::npos) {
		
			(*msg).clear();
			return;
		
		}

		std::string callSign = my::toUpper((*msg).substr(idx+1));
		cout << callSign << endl;
		printCallSignCountry(callSign);
		(*msg).clear();
		return;

	}


	idx = (*msg).find(' ');
	if (idx == std::string::npos) {
		cout << "ERR: No frequency specified" << endl;
		(*msg).clear();
		return;
	}

	// pick off the frequency (or command)
	std::string freq = my::toUpper((*msg).substr(0, idx));

	// and the message (or argument)
	(*msg) = my::toUpper(my::strip((*msg).substr(idx + 1)));

	// handle commands
	if (freq == "LEVEL") {

		int level = atoi((*msg).c_str());
		if (level > 0 && level <= 100) {

			(*audio).setVolume(static_cast<float>(level) / 100.0);
			cout << "OK: Level now " << level << "%" << endl;

		} else {

			cout << "ERR: Invalid level provided; must be 1 to 100" << endl;

		}
		(*msg).clear();

		return;

	} else if (freq == "DEPTH") {

		int level = atoi((*msg).c_str());

		if (level >= 1 && level <= 3) {
		
			(*audio).setDepth(level);
		
			cout << "OK: Depth now " << level << endl;
		} else {
		
			cout << "ERR: Invalid depth provided; must be 1 to 3" << endl;
		
		}

		(*msg).clear();
		return;

	}

	// handle even/odd
	TimeSlots eo = NextSlot;
	if (freq.size()) {

		char eoc = freq[freq.size() - 1];

		if (!isdigit(eoc)) {

			freq = freq.substr(0, freq.size() - 1);
			switch(eoc) {
				case 'E':
				case 'e':
					eo = EvenSlot;
					break;
				case 'O':
				case 'o':
					eo = OddSlot;
					break;
				default:
					//
					//  TODO: report this as an error
					//
					break;
			}
		}

	}

	// read the frequency
	double f = atof(freq.c_str());
	if (f > 0) {

		cout << "OK: Send @ " << f << "Hz: '" << msg << "'" << endl;
		(*audio).transmit((*msg), f, eo);
		(*msg) = "";

	} else {

		cout << "ERR: Invalid frequency specified" << endl;
		
	}

}


//
//  usage()
//
void usage(const std::string &s) {
	cerr << endl;
	cerr << "Usage: " << s << " <mode> <device> [depth]" << endl;
	cerr << endl;
	SoundCard::showDevices();
}



//
// Decode Messages Viewer Handler
// 
void handleDecodedMessages(vector<DecodedLine> * newMessagesPtr)
{

	char tmpMsg[128];
	char *cqFoundPtr = 0;

	tmpMsg[0] = '\0';

	if (newMessagesPtr != 0 && (*newMessagesPtr).size() > 0 ) {
	
		for (long unsigned int i = 0; i < (*newMessagesPtr).size(); i++) {
		
			if (cacheDecodedMessages.size() > MAX_DECODED_MESSAGES) {

				cacheDecodedMessages.erase(cacheDecodedMessages.end());
			
			}

						

			if (cqOnlyEnabled == true) 
			{
				tmpMsg[0] = '\0';
				sprintf(tmpMsg,"%s",(*newMessagesPtr)[i].getContent().c_str());
			
				cqFoundPtr = strstr( tmpMsg, "CQ " );
				if (cqFoundPtr == NULL) {
					continue;
				}
			}
			

			cacheDecodedMessages.insert(cacheDecodedMessages.begin(), (*newMessagesPtr)[i]);

			cerr << (*newMessagesPtr)[i].getContent().c_str() << endl;

			decodedMessageQt++;

		}
		
	} 

}


// 
// Clean decoded messages cache
//

void wipeDecodedMessages() 
{
	cacheDecodedMessages.clear();
	cout << "Decoded messages cache cleanned" << endl;
}


// 
// Print decoded messages on demand
//

void printDecodedMessages()
{
		
	char fixedLine[64];
	char csvLine[128];
	char tmpMsg[128];

	fixedLine[0] = '\0';
	csvLine[0] = '\0';
	tmpMsg[0] = '\0';

	int qtDM = 0;
	char *decodedMessagePtr = 0;
	char *msgContent = 0;
	char *cqFoundPtr = 0;

	for (const auto message: cacheDecodedMessages) {

		sprintf(tmpMsg,"%s",message.getContent().c_str());

		csvLine[0] = '\0';
		msgContent = const_cast<char*>(tmpMsg);

		decodedMessagePtr = strtok(msgContent, " ");
		if(decodedMessagePtr != 0)
		{
			//dB
			strcat(csvLine, decodedMessagePtr);
			strcat(csvLine, ";");
			decodedMessagePtr = strtok(NULL, " ");

		} else {

			strcat(csvLine, "-;");

		}

		if(decodedMessagePtr != 0)
		{
			//dt
			strcat(csvLine, decodedMessagePtr);
			strcat(csvLine, ";");
			decodedMessagePtr = strtok(NULL, " ");
		} else {

			strcat(csvLine, "-;");

		}

		if(decodedMessagePtr != 0)
		{
			//Frequency
			strcat(csvLine, decodedMessagePtr);
			strcat(csvLine, ";");
			decodedMessagePtr = strtok(NULL, " ");
		
		} else {

			strcat(csvLine, "-;");

		}

		//Ignored
		if(decodedMessagePtr != 0)
		{
			decodedMessagePtr = strtok(NULL, " ");
		}
		if(decodedMessagePtr != 0) {

			//Souce Call Sign or CQ
			strcat(csvLine, decodedMessagePtr);
			decodedMessagePtr = strtok(NULL, " ");
		} else {

			strcat(csvLine, "-;");

		}

		if(decodedMessagePtr != 0) {

			if (strlen(decodedMessagePtr) <= 5)
			{

				strcat(csvLine, " ");
				strcat(csvLine, decodedMessagePtr);
				strcat(csvLine, ";");
				decodedMessagePtr = strtok(NULL, " ");

			} else {
				strcat(csvLine, ";");
			}

			if (decodedMessagePtr != 0) {
				//Destination Call Sign
				strcat(csvLine, decodedMessagePtr);
				strcat(csvLine, ";");
				decodedMessagePtr = strtok(NULL, " ");
			
			} else {

				strcat(csvLine, "-;");

			}

		} else {

			strcat(csvLine, "-;");

		}

		if(decodedMessagePtr != 0) {

			//Grid Square Locator or Rec Signal
			strcat(csvLine, decodedMessagePtr);
			strcat(csvLine, ";");

		} else {

			strcat(csvLine, "-;");

		}

		sprintf(fixedLine,"%10ld;%.36s\n\r",message.getTime(), csvLine);

		send(new_socket, fixedLine, strlen(fixedLine), 0);

		cout << "Command response:" << fixedLine << endl;

		qtDM ++;
	}

    if (qtDM == 0)
	{
		sprintf(fixedLine,"EMPTY\n\r");
		send(new_socket, fixedLine, strlen(fixedLine), 0);
		
	}
	
}


// 
// Async decode reading
//
void *asyncDecodeMessage(void * arg)
{
	
	while (true > 0) {

		// process messages in the decoder
		vector<DecodedLine> *decLinesPtr = ((ModemSoundDevice *)arg)->run();
		handleDecodedMessages(decLinesPtr);
		delete decLinesPtr;
                
	}

	pthread_exit(NULL);
}


// EOF
