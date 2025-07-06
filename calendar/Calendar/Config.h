#ifndef _CONFIG_H
#define _CONFIG_H

#define LED_PIN_ESP32 21

//Touch pin (10 = T10)
//Treshold: lower values, more sensitive
#define TOUCH_PIN_WAKEUP 10
#define TOUCH_THRESHOLD 5000

//SPI Pin config (for both displays)
#define SPI_PIN_SCK  17
#define SPI_PIN_MOSI 15
#define SPI_PIN_MISO 16 //Not used

//Pin config top display
#define TOP_PIN_CS   8
#define TOP_PIN_DC   14
#define TOP_PIN_RST  13
#define TOP_PIN_BUSY 5
#define TOP_PIN_PWR  12

//Pin config bottom display
#define BOTTOM_PIN_CS   38
#define BOTTOM_PIN_DC   7
#define BOTTOM_PIN_RST  18
#define BOTTOM_PIN_BUSY 6
#define BOTTOM_PIN_PWR  9

//Config below is for test purposes
/*
#define LED_PIN_ESP32 2

//SPI Pin config (for both displays)
#define SPI_PIN_SCK  18
#define SPI_PIN_MOSI 23
#define SPI_PIN_MISO 19 //Not used

//Pin config top display
#define TOP_PIN_CS   5
#define TOP_PIN_DC   27
#define TOP_PIN_RST  14
#define TOP_PIN_BUSY 12
#define TOP_PIN_PWR  13

//Pin config bottom display
//Copy of top - testing is done with only one display, so the same pin config
#define BOTTOM_PIN_CS   5
#define BOTTOM_PIN_DC   27
#define BOTTOM_PIN_RST  14
#define BOTTOM_PIN_BUSY 12
#define BOTTOM_PIN_PWR  13
*/

#endif
