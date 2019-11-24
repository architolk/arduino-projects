//File should be a 8 unsigned bit, 44.100 kHz mono audio file
File dataFile;

void setup(){

  //DAC 0..7 Pins as output
  for (byte i=0;i<8;i++){
    pinMode(i, OUTPUT);//set digital pins 0-7 as outputs
  }

  if(!SD.begin(SD_ChipSelectPin))
  {
    return;
    //SD card fails, should be some led or whatever
  }

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

}

ISR(TIMER1_COMPA_vect){//timer1 interrupt ~44.1kHz to send audio data (it is really 44.199kHz)
  if (dataFile) {
    if (dataFile.available()) {
      //Send byte to port D;
      PORTD = dataFile.read();
    } else {
      dataFile.close();
      openFile(); //Loop file
    }
}


void loop(){
  //do other stuff here
}

void openFile() {
  dataFile = SD.open("sample.wav");
  if (!dataFile) {
    //Some error or LED or whatever: error reading file
  }
}
