# FT8MODEM FOR NETWORK 

Network service modem application for FT8, forked from KK5JY original project and creator:

http://www.kk5jy.net/ft8modem/

I've forked this project and added network support primary to run my 8 Bit FT8 Client for MSX (Z80 based) computers

If you are into retrocomputing, check this project also :) :

https://github.com/CleversonSA/ft8msxclient



# PREREQUISITES

This project depends on the 'rtaudio' package, available here:

    https://www.music.mcgill.ca/~gary/rtaudio/

Please follow the instructions there for installation.

ATTENTION: This fork uses the 5.2.0 version of this package. You will need download this tag version. Newer versions are incompatible and will give errors when compiling the ft8modem (Oct/2023).

Also you need install WSJT-X first, because the jt9 program embedded into the application. This is the heart of decoding signals and ft8modem heavily uses it. To compile and install, follow the instructions in WSJT-X home page:

   https://wsjt.sourceforge.io/wsjtx.html

 

# INSTALLATION

To build the modem application, run at the command line:

    $ make

To install to /usr/local/bin, run:

    $ make install



# RUNNING

To run the application:

    $ ft8modem

It will list the available sound card devices (described into Device ID number). Example:

    Usage: ./ft8modem <mode> <device> [depth]

    Valid devices:
        + Device ID = 0: "default", inputs = Multi, outputs = Multi, rates = 4000, 5512, 8000, 9600, 11025, 16000, 22050, 32000, 44100, 48000, 88200, 96000, 176400, 192000

Choose the correct sound card and run (example Device ID #0):

    $ ft8modem ft8 0

It will open the 6666 TCP port, so you can telnet it:

    $ telnet localhost 6666

Run a command:

    logs[PRESS ENTER]



# TCP NETWORK COMMANDS

As a network service, It will be decoding FT8 signals in background, keep a internal memory log of decoded messages.

You can transmit also, but it will be available soon:

    - LOGS\n\r

        Return the list of decoded messages in memory.

        Returns: 

            + CSV (';' separated lines ended by \n\r) list of decoded messages from memory buffer. It will return 'EMPTY\n\r' if no decoded messages are available
    

    - WIPE\n\r 

        Clears the decoded message memory.

        Returns:

            None


    - CQONLYENABLED\n\r

        Clear the decoded messages and only accept CQ Calls. Good when the band is full of activity.

        Returns:

            None


    - CQONLYDISABLED\n\r

        Clear the decoded messages and accept ALL FT8 band activity. Good to check a band propagation.

        Returns:

            None


    - QRZCOUNTRY <Call Sign>\n\r

        Try to identify the country of a call sign, based on http://www.arrl.org/international-call-sign-series list.

        Returns:

            Country of a call sign ended by \n\r

        

# LICENSE

    - GPL v3
