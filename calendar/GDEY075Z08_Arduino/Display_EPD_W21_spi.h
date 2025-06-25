#ifndef _DISPLAY_EPD_W21_SPI_
#define _DISPLAY_EPD_W21_SPI_
#include "Arduino.h"

//IO settings
//SCK--GPIO23(SCLK)
//SDIN---GPIO18(MOSI)
#define isEPD_W21_BUSY digitalRead(12)  //BUSY
#define EPD_W21_RST_0 digitalWrite(14,LOW)  //RES
#define EPD_W21_RST_1 digitalWrite(14,HIGH)
#define EPD_W21_DC_0  digitalWrite(27,LOW) //DC
#define EPD_W21_DC_1  digitalWrite(27,HIGH)
#define EPD_W21_CS_0 digitalWrite(5,LOW) //CS
#define EPD_W21_CS_1 digitalWrite(5,HIGH)


void SPI_Write(unsigned char value);
void EPD_W21_WriteDATA(unsigned char datas);
void EPD_W21_WriteCMD(unsigned char command);


#endif 
