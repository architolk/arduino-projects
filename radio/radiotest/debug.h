//Uncomment to enable debug (writing stuff to serial)
//#define DEBUG

#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <Wire.h>

#ifdef DEBUG
  #define DebugBegin(__info) Serial.begin(__info)
	#define Debug(__info) Serial.print(__info)
  #define Debugln(__info) Serial.println(__info)
#else
  #define DebugBegin(__info)
	#define Debug(__info)
  #define Debugln(__info)
#endif

#endif
