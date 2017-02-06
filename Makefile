# Makefile

CC       = g++
CFLAGS   = -std=c++11 -DRASPBERRY_PI -DBCM2835_NO_DELAY_COMPATIBILITY -D__BASEFILE__=\"$*\"
LIBS     = -lbcm2835
LMICBASE = ../../../src
INCLUDE  = -I$(LMICBASE)

all: otaa_test

raspi.o: $(LMICBASE)/raspi/raspi.cpp
                                $(CC) $(CFLAGS) -c $(LMICBASE)/raspi/raspi.cpp $(INCLUDE)

radio.o: $(LMICBASE)/lmic/radio.c
                                $(CC) $(CFLAGS) -c $(LMICBASE)/lmic/radio.c $(INCLUDE)

oslmic.o: $(LMICBASE)/lmic/oslmic.c
                                $(CC) $(CFLAGS) -c $(LMICBASE)/lmic/oslmic.c $(INCLUDE)

lmic.o: $(LMICBASE)/lmic/lmic.c
                                $(CC) $(CFLAGS) -c $(LMICBASE)/lmic/lmic.c $(INCLUDE)

hal.o: $(LMICBASE)/hal/hal.cpp
                                $(CC) $(CFLAGS) -c $(LMICBASE)/hal/hal.cpp $(INCLUDE)

aes.o: $(LMICBASE)/aes/lmic.c
                                $(CC) $(CFLAGS) -c $(LMICBASE)/aes/lmic.c $(INCLUDE) -o aes.o

otaa_test.o: otaa_test.cpp
                                $(CC) $(CFLAGS) -c $(INCLUDE) $<

otaa_test: otaa_test.o raspi.o radio.o oslmic.o lmic.o hal.o aes.o
                                $(CC) $^ $(LIBS) -o otaa_test

clean:
                                rm -rf *.o otaa_test

