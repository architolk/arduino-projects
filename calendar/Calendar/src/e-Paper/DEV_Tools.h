#ifndef _DEV_TOOLS_H_
#define _DEV_TOOLS_H_

#include <Arduino.h>
#include <stdint.h>
#include <stdio.h>
#include <SPI.h>

/**
 * data
**/
#define UBYTE   uint8_t
#define UWORD   uint16_t
#define UDOUBLE uint32_t


/**
 * GPIO read and write
**/
#define DEV_Digital_Write(_pin, _value) digitalWrite(_pin, _value == 0? LOW:HIGH)
#define DEV_Digital_Read(_pin) digitalRead(_pin)

/**
 * delay x ms
**/
#define DEV_Delay_ms(__xms) delay(__xms)

/*------------------------------------------------------------------------------------------------------*/
void DEV_SPI_WriteByte(UBYTE data);

#endif
