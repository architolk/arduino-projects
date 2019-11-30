/*
enhanced-sampler.ino

Interrupt timer used to output a wave file to PORTA
On a MEGA PORTA is pins 22-29
PORTA is connected to a DAC
For Uno use REPLACE PORTA WITH PORTD (pins 0-7)

Least frequency deviation achieved by utilising Timer 2 interrupt
With 8 prescalar

Timer 2 has a finite resolution as it is ultimately dependent on integer values
set in a single byte: OCR2A = setocroa where 0 >= setocra <=255
Thus requested frequencies may be a best fit-
Some will be better than others- its a modular maths factor.

Timer 1 - the frequency increments are too big
(Small changes in requested frequency can not be reflected in interrupt period)
Timer 0 appears to affect millis()

Acknowledgment Amanda Ghassaei
Ref http://www.instructables.com/id/Arduino-Timer-Interrupts/
*/

// Defines for clearing register bits
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
// Defines for setting register bits
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

#define SD_ChipSelectPin 10

#include <SD.h>
#include <SPI.h>
// set up variables using the SD utility library functions:
Sd2Card card;
boolean hascard=false;
File tempfile;
char bufFile[]="sample.wav";
unsigned long starttime, stoptime, counter, readings;
float frequency=9300;
boolean finished=false;
#define BUF_SIZE 512
uint8_t bufa[BUF_SIZE];
uint8_t bufb[BUF_SIZE];
uint16_t bufcount;
byte headbuf[60]; // will read the header into this

boolean aready, readit;

void setup(){
  pinMode(SD_ChipSelectPin, OUTPUT); //Don't know if this is actually necessary

  // SERIAL: For debugging
  Serial.begin(115200); // start serial for output
  Serial.flush();
  Serial.println(F("\nFast D/A"));
  // ---

  // On UNO PORTD is pins 0-7
  // set digital pins PORTD as output
  DDRD = B11111111;

  // Set port to half voltage = silence
  PORTD=128;

  if (SD.begin(SD_ChipSelectPin)){
    hascard=true;
    card.init(SPI_FULL_SPEED, SD_ChipSelectPin);
  } else {

    // SERIAL: For debugging
    Serial.println(F("SD card problem!"));
    // Should be doing something else: making a led flash or whatever...
    // ---

    while(1); // no point in continuing
  }

  getfile();
  frequency=fileopen();
  if (frequency==0) {

    // SERIAL: For debugging
    Serial.println(F("\nFrequency problem"));
    // Make a led flash to indicate frequency problem
    // ---

  }
  readings=datasize();
  if (datasize==0) {

    // SERIAL: For debugging
    Serial.println(F("\ndata size problem"));
    // Make a led flash to indicate datasize problem
    // ---

  }

  if((frequency >0) && (datasize>0)) {
    setintrupt(frequency);
  }
}

void setintrupt(float freq){
  float bitval=8; // 8 for 8 bit timers 0 and 2, 1024 for timer 1
  byte setocroa=(16000000/(freq*bitval))-0.5; // -1 +0.5
  // The resolution of the timer is limited-
  // Ultimately determined by magnitude of bitval

  bufcount=0; // initialise counters
  counter=0;

  //SERIAL: For debugging
  Serial.println(F("Outputting File to PORT"));
  //---

  cli();//disable interrupts
  //set timer2 interrupt
  TCCR2A = 0;// set entire TCCR2A register to 0
  TCCR2B = 0;// same for TCCR2B
  TCNT2  = 0;//initialize counter value to 0
  // set compare match register for frequency (hz) increments
  OCR2A = setocroa;// = (16*10^6) / (frequency*8) - 1 (must be <256)
  // turn on CTC mode
  TCCR2A |= (1 << WGM21);
  // Set CS21 bit for 8 prescaler
  TCCR2B |= (1 << CS21);
  // enable timer compare interrupt
  //  TIMSK2 |= (1 << OCIE2A);
  sbi(TIMSK2,OCIE2A); //  enable interrupt on timer 2
  aready=true;
  readit=true; // need to start reading bufb!
  starttime=millis();
  sei();//enable interrupts
}

// interrupt routine! ***************************************
ISR(TIMER2_COMPA_vect){ //interrupt routine timer 2
  if (counter<readings) {
    if(aready) {
      PORTD=bufa[bufcount];
    } else {
      PORTD=bufb[bufcount];
    }
    counter++;
    bufcount++;
    if (bufcount==BUF_SIZE){
      if(readit==false) { // file reading not going on
        bufcount=0;
        aready = ! aready;
        readit=true;
      } else {
        // file reading still going on so backup and wait
        counter--;
        bufcount--;
      }
    }
  } else {
    cli();
    stoptime = millis();
    cbi(TIMSK2,OCIE2A); // disable interrupt
    PORTD=128;
    finished=true;
    tempfile.close();
    sei();
  }
}
// End interrupt ********************************************

void loop() {
  //Reading of SD card is performed seperate from timer interupt
  //triggered by readit = true
  if(readit) {
    if (!aready) {
      // initiate SDCard block read to bufa
      tempfile.read(bufa, BUF_SIZE);
    } else {
      // initiate SDCard block read to bufb
      tempfile.read(bufb, BUF_SIZE);
    }
    readit=false;
  }
}

void getfile(){

  //SERIAL: For debugging
  //getFile should be something, maybe from buttons. Now defaults to default bufFile
  Serial.print(F("File selected "));
  Serial.println(bufFile);
  //---

}

unsigned long fileopen() {
  unsigned long retval;
  tempfile = SD.open(bufFile, FILE_READ);
  if(!tempfile){
    return(0);
  } else {
    // read frequency specified in wav file header
    tempfile.seek(0);
    tempfile.read(headbuf,60);
    retval=headbuf[27];
    retval=(retval<<8) | headbuf[26];
    retval=(retval<<8) | headbuf[25];
    retval=(retval<<8) | headbuf[24];

    //SERIAL: For debugging
    Serial.print(F("File Frequency "));
    Serial.print(retval);
    //---

    return(retval);
  }
}

unsigned long datasize() {
  // read data size specified in wav file header
  unsigned long retval;

  // look for the word "data" in header
  // some software adds optional fields to the header
  // altering the position of the data start and data length fields
  // but expect to find the data length  and data, after the word "data"

  int mypos=40;
  for (int i=36; i<60;i++) {
    if (headbuf[i] == 'd') {
      if(headbuf[i+1]=='a') {
        if(headbuf[i+2]=='t') {
          if(headbuf[i+3]=='a') {
            // at last we have it
            mypos=i+4;
            i=60;
          }
        }
      }
    }
  }
  tempfile.seek(mypos);
  retval=headbuf[mypos+3];
  retval=(retval<<8) | headbuf[mypos+2];
  retval=(retval<<8) | headbuf[mypos+1];
  retval=(retval<<8) | headbuf[mypos];
  tempfile.seek(mypos+4);
  unsigned long nowtime,endtime;

  nowtime=micros();
  // Read 1st block of data
  tempfile.read(bufa, BUF_SIZE);
  endtime=micros();

  //SERIAL: For debugging
  Serial.print(F(" , Data size "));
  Serial.print(retval);
  Serial.print(F("b Data speed "));
  Serial.print(BUF_SIZE);
  Serial.print(F("b in "));
  Serial.print(float(endtime-nowtime)/1000,2);
  Serial.println(F(" mS"));
  Serial.print(F("Sample period "));
  Serial.print(1000/frequency,2);
  Serial.println(F(" mS"));
  //---

  return(retval);
}
