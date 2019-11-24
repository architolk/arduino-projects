//Timer2 is used, for sampling rate of 8000Hz
//Faster sampling rate is better, but we need to have a sample for that speed
//And we need to check at what speed the SD card can be read

// Import the sample at 8000Hz
const unsigned char sample[] PROGMEM = {127};
long index = 0;

void setup() {

  cli();//stop interrupts

  //set timer2 interrupt at 8kHz
  TCCR2A = 0;// set entire TCCR2A register to 0
  TCCR2B = 0;// same for TCCR2B
  TCNT2  = 0;//initialize counter value to 0
  // set compare match register for 8khz increments
  OCR2A = 249;// = (16*10^6) / (8000*8) - 1 (must be <256)
  // turn on CTC mode
  TCCR2A |= (1 << WGM21);
  // Set CS21 bit for 8 prescaler
  TCCR2B |= (1 << CS21);
  // enable timer compare interrupt
  TIMSK2 |= (1 << OCIE2A);

  sei();//enable interrupts

}

void loop() {
  //Nothing to do, everything is done in the timers
}

//Timer 2 interupt at 8000Hz
ISR(TIMER2_COMPA_vect){
  PORTD = pgm_read_byte(&(sample[index++]));
  if (index>=sizeof(sample)) {
    index = 0;
  }
}
