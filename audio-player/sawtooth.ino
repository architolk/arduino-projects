//mono saw out with 44.1kHz sampling rate
//by Amanda Ghassaei
//Nov 2012
//https://www.instructables.com/id/Stereo-Audio-with-Arduino/

/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
*/

//Timer works at 44.1kHz sampling rate, so at timer1 is used

//control pins of TLC7528
#define outputSelector 8
#define CS 9
#define WR 10

byte saw = 0;//value of saw wave

void setup(){

  for (byte i=0;i<8;i++){
    pinMode(i, OUTPUT);//set digital pins 0-7 as outputs
  }

  pinMode(outputSelector,OUTPUT);
  pinMode(CS,OUTPUT);
  pinMode(WR,OUTPUT);

  cli();//stop interrupts

  //set timer1 interrupt at ~44.1kHz
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  // set compare match register for 1hz increments
  OCR1A = 361;// = (16*10^6) / (44100*1) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS10 bit for 1 prescaler
  TCCR1B |= (1 << CS10);
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);

  sei();//enable interrupts

  //select output (LOW for DACA and HIGH for DACB)
  digitalWrite(outputSelector,LOW);

  //set CS and WR low to let incoming data from digital pins 0-7 go to DAC output
  digitalWrite(CS,LOW);
  digitalWrite(WR,LOW);

}

ISR(TIMER1_COMPA_vect){//timer1 interrupt ~44.1kHz to send audio data (it is really 44.199kHz)
  PORTD = saw;//send saw out to the DAC through digital pins 0-7
  saw++;//increment saw value by one
  if (saw==256){//reset saw if it reaches 256 (keeps output within 0-255 always)
    saw=0;
  }
}


void loop(){
  //do other stuff here
}
