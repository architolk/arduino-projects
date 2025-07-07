#include "EInkDisplay.h"
#include "../Debug.h"

EInkDisplay::EInkDisplay(uint8_t pinCS, uint8_t pinDC, uint8_t pinRST, uint8_t pinBUSY, uint8_t pinPWR) {
  _pinCS = pinCS;
  _pinDC = pinDC;
  _pinRST = pinRST;
  _pinBUSY = pinBUSY;
  _pinPWR = pinPWR;
}

void EInkDisplay::setupPins() {
  pinMode(_pinBUSY,  INPUT);
  pinMode(_pinRST , OUTPUT);
  pinMode(_pinDC  , OUTPUT);
  pinMode(_pinPWR  , OUTPUT);
  pinMode(_pinCS , OUTPUT);

  digitalWrite(_pinPWR , LOW); // Display receives no power
  digitalWrite(_pinCS , LOW); // Do not select this display for SPI
}

void EInkDisplay::activate() {
  active = true;
  digitalWrite(_pinPWR , HIGH); // Display receives power
  digitalWrite(_pinCS , HIGH); // Select this display for SPI
  DEV_Delay_ms(100); // Give the display some time to listen
}

void EInkDisplay::deactivate() {
  active = false;
  digitalWrite(_pinPWR , LOW); // Display receives no power any more
  digitalWrite(_pinCS , LOW); // Do not select this display for SPI
}

void EInkDisplay::init() {
  if (active) {
    EPD_7IN5B_V2_Reset();

    EPD_7IN5B_V2_SendCommand(0x01);			//POWER SETTING
    EPD_7IN5B_V2_SendData(0x07);
    EPD_7IN5B_V2_SendData(0x07);    //VGH=20V,VGL=-20V
    EPD_7IN5B_V2_SendData(0x3f);		//VDH=15V
    EPD_7IN5B_V2_SendData(0x3f);		//VDL=-15V

    //Enhanced display drive(Add 0x06 command)
    EPD_7IN5B_V2_SendCommand(0x06);			//Booster Soft Start
    EPD_7IN5B_V2_SendData(0x17);
    EPD_7IN5B_V2_SendData(0x17);
    EPD_7IN5B_V2_SendData(0x28);
    EPD_7IN5B_V2_SendData(0x17);

    EPD_7IN5B_V2_SendCommand(0x04); //POWER ON
    DEV_Delay_ms(100);
    EPD_7IN5B_V2_WaitUntilIdle();

    EPD_7IN5B_V2_SendCommand(0X00);			//PANNEL SETTING
    EPD_7IN5B_V2_SendData(0x0F);   //KW-3f   KWR-2F	BWROTP 0f	BWOTP 1f

    EPD_7IN5B_V2_SendCommand(0x61);        	//tres
    EPD_7IN5B_V2_SendData(0x03);		//source 800
    EPD_7IN5B_V2_SendData(0x20);
    EPD_7IN5B_V2_SendData(0x01);		//gate 480
    EPD_7IN5B_V2_SendData(0xE0);

    EPD_7IN5B_V2_SendCommand(0X15);
    EPD_7IN5B_V2_SendData(0x00);

    EPD_7IN5B_V2_SendCommand(0X50);			//VCOM AND DATA INTERVAL SETTING
    EPD_7IN5B_V2_SendData(0x11);
    EPD_7IN5B_V2_SendData(0x07);

    EPD_7IN5B_V2_SendCommand(0X60);			//TCON SETTING
    EPD_7IN5B_V2_SendData(0x22);
  }
}

void EInkDisplay::sleep(void) {
  if (active) {
    EPD_7IN5B_V2_SendCommand(0X02);  	//power off
    EPD_7IN5B_V2_WaitUntilIdle();
    EPD_7IN5B_V2_SendCommand(0X07);  	//deep sleep
    EPD_7IN5B_V2_SendData(0xA5);
  }
}

void EInkDisplay::clear() {
  if (active) {
    UWORD Width, Height;
    Width =(EPD_7IN5B_V2_WIDTH % 8 == 0)?(EPD_7IN5B_V2_WIDTH / 8 ):(EPD_7IN5B_V2_WIDTH / 8 + 1);
    Height = EPD_7IN5B_V2_HEIGHT;

    UWORD i;
    EPD_7IN5B_V2_SendCommand(0x10);
    for(i=0; i<Width*Height; i++) {
        EPD_7IN5B_V2_SendData(0xff);

    }
    EPD_7IN5B_V2_SendCommand(0x13);
    for(i=0; i<Width*Height; i++)	{
        EPD_7IN5B_V2_SendData(0x00);

    }
    EPD_7IN5B_V2_TurnOnDisplay();
  }
}

//0 is black area, 1 is red area
void EInkDisplay::writeCanvas(GFXcanvas1 *_canvas, UBYTE Block) {
  if (active) {

  	UDOUBLE Width, Height;
  	Width =(EPD_7IN5B_V2_WIDTH % 8 == 0)?(EPD_7IN5B_V2_WIDTH / 8 ):(EPD_7IN5B_V2_WIDTH / 8 + 1);
  	Height = EPD_7IN5B_V2_HEIGHT;

  	if (Block == 0) {
  	    EPD_7IN5B_V2_SendCommand(0x10);
  	} else {
  		  EPD_7IN5B_V2_SendCommand(0x13);
  	}
  	for (UDOUBLE h = 0; h < Height; h++) {
        for (UDOUBLE w = 0; w < Width; w++) {
            byte pixel_buffer;
            for (int b = 0; b < 8; b++) {  // get 8 pixel and stuff them into one byte
                pixel_buffer = pixel_buffer << 1;
                pixel_buffer += _canvas->getPixel(w * 8 + b, h);
            }
            EPD_7IN5B_V2_SendData(pixel_buffer);  // write the byte to the display
        }
    }

    if(Block == 1){
  		EPD_7IN5B_V2_TurnOnDisplay();
  	}
  }
}

void EInkDisplay::EPD_7IN5B_V2_Reset() {
  if (active) {
    DEV_Digital_Write(_pinRST, 1);
    DEV_Delay_ms(200);
    DEV_Digital_Write(_pinRST, 0);
    DEV_Delay_ms(2);
    DEV_Digital_Write(_pinRST, 1);
    DEV_Delay_ms(200);
  }
}

void EInkDisplay::EPD_7IN5B_V2_SendCommand(UBYTE Reg) {
  if (active) {
    DEV_Digital_Write(_pinDC, 0);
    DEV_Digital_Write(_pinCS, 0);
    DEV_SPI_WriteByte(Reg);
    DEV_Digital_Write(_pinCS, 1);
  }
}

void EInkDisplay::EPD_7IN5B_V2_SendData(UBYTE Data) {
  if (active) {
    DEV_Digital_Write(_pinDC, 1);
    DEV_Digital_Write(_pinCS, 0);
    DEV_SPI_WriteByte(Data);
    DEV_Digital_Write(_pinCS, 1);
  }
}

void EInkDisplay::EPD_7IN5B_V2_WaitUntilIdle() {
  if (active) {
  	Debugln("e-Paper busy");
  	unsigned char busy;
  	do	{
  		EPD_7IN5B_V2_SendCommand(0x71);
  		busy = DEV_Digital_Read(_pinBUSY);
  		busy =!(busy & 0x01);
  	}while(busy);
  	DEV_Delay_ms(200);
  	Debugln("e-Paper busy release");
  }
}

void EInkDisplay::EPD_7IN5B_V2_TurnOnDisplay() {
  if (active) {
    EPD_7IN5B_V2_SendCommand(0x12);			//DISPLAY REFRESH
    DEV_Delay_ms(100);	        //!!!The delay here is necessary, 200uS at least!!!
    EPD_7IN5B_V2_WaitUntilIdle();
  }
}
