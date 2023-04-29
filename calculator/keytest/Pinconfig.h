#ifndef _PINCONFIG_h
#define _PINCONFIG_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

// KEYCHECK = interupts on input pins check on keypress
// SWPROBE = output pin for checking switches & sliders

#define PIN_KEYCHECK1 36 // Pin 11 @ calculator
#define PIN_KEYCHECK2 39 // Pin 13
#define PIN_KEYCHECK3 34 // Pin 15
#define PIN_KEYCHECK4 35 // Pin 16
#define PIN_KEYCHECK5 32 // Pin 20
#define PIN_KEYCHECK6 33 // Pin 22
#define PIN_AUDIO 25
#define PIN_KEYCHECK7 26 // Pin 24
#define PIN_KEYCHECK8 27 // Pin 25
#define PIN_SWPROBE1 14 // Pin 5 @ calculator
#define PIN_SWPROBE2 12 // Pin 6
#define PIN_SWPROBE3 21 // Pin 7
#define PIN_LED 22 // Internal LED pin
#define PIN_VFD_RST 17 // VFD reset
#define PIN_SWCHECK1 16 // Pin 1 @ calculator
#define PIN_SWCHECK2 4 // Pin 2
#define PIN_KEYPROBE1 0 // Pin 8
#define PIN_SWCHECK3 2 // Pin 3
#define PIN_KEYPROBE2 15 // Pin 9
#define PIN_KEYPROBE3 13 // Pin 10
/*
// Tijdelijk uitgezet - deze pinnen zijn voor Seriele communicatie nodig
#define PIN_KEYPROBE4 1 // Pin 11
#define PIN_SWCHECK4 3 // Pin 4
*/
// Om de serial communicatie mogelijk te houden (!)
#define PIN_KEYPROBE4 21 // Pin 11 - tijdelijk voor testdoeleinden - SWPROBE3 and KEYPROBE4 double in this scenario
#define PIN_SWCHECK4 25 // Pin 4 - tijdelijk voor testdoeleinden

struct KeyRaw {
	int key1,key2,key3,key4;
};

class PINCONFIG {
  public:
    void init();
    int testDecimals();
    bool testPaperFeed();
    bool testRound();
    bool testK();
    int testItem();
    bool testPrint();
    bool testAM();
    bool testKeys(KeyRaw &keys);
    bool testKeysDelay(int delayTime, KeyRaw &keys);

	private:
		int testKeyRow();
};

extern PINCONFIG Pinconfig;

#endif
