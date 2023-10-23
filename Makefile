#
#
#   Makefile
#
#   Copyright (C) 2023 by Matt Roberts.
#   License: GNU GPL2 (www.gnu.org)
#
#

TARGETS1=ft8modem ft8encode test_decode
TARGETS=$(TARGETS1)
OBJECTS1=nlimits.o call_sign_driver.o
LIBS1=-lm -L/usr/local/bin -lrtaudio -lsndfile -lpthread
BINDIR=/usr/local/bin

CDEBUG=-Wall -ggdb -D_DEBUG

all: $(TARGETS)

.cpp.o:
	g++ -Wall $(CFLAGS) $(CDEBUG) -c $<
.cc.o:
	g++ -Wall $(CFLAGS) $(CDEBUG) -c $<

$(TARGETS1): %: %.o $(OBJECTS1)
	g++ $(CFLAGS) $(CDEBUG) -o $@ $@.o $(OBJECTS1) $(LIBS1) $(LIBS2)

# meta-targets
clean:
	rm -f $(TARGETS) core *.o *.wav
	rm -f Makefile.bak decoded.txt jt9_wisdom.dat timer.out avemsg.txt
	rm -rf __pycache__

strip: $(TARGETS)
	strip --strip-unneeded $(TARGETS)

rebuild:
	make -j1 clean
	make

install: strip
	for i in $(TARGETS) ; do install -o root -g root -m 0755 $$i $(BINDIR) ; done

dep depend:
	makedepend -Y *.cc *.cpp 2>/dev/null

# EOF
# DO NOT DELETE

call_sign_driver.o: call_sign_driver.h
ft8encode.o: sf.h mfsk.h shape.h nlimits.h IFilter.h osc.h es.h
ft8modem.o: snddev.h sc.h mfsk.h shape.h nlimits.h IFilter.h osc.h es.h 
ft8modem.o: decode.h sf.h stype.h clock.h FirFilter.h WindowFunctions.h
ft8modem.o: FilterTypes.h FilterUtils.h 
test_decode.o: decode.h sf.h stype.h clock.h
nlimits.o: nlimits.h
