#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <Wire.h>

#ifdef DEBUG
	#define Debug(__info) Serial.print(__info)
  #define Debugln(__info) Serial.println(__info)
#else
	#define Debug(__info)
  #define Debugln(__info)
#endif

#endif
