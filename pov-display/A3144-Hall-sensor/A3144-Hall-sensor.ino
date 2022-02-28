// Test for the hall sensor
// 
// This will toggle between an ON en OFF for the LED

int led = 4;
int hall = 2;

boolean state = HIGH;

volatile unsigned long rotationTime, timeOld, timeNew;

unsigned long currentRotationTime,spinOld,spinNew = 0;

volatile byte pulse = 0;

// the setup routine runs once when you press reset:
void setup() {
  // initialize the digital pin as an output.
  pinMode(led, OUTPUT);
  pinMode(hall, INPUT_PULLUP);
  timeNew = micros();
  timeOld = timeNew;
  attachInterrupt(digitalPinToInterrupt(hall), magnetPresent, FALLING);
}

// the loop routine runs over and over again forever:
void loop() {
  spinNew = micros();
  if (spinNew-spinOld>currentRotationTime) {
    spinOld = spinNew;
    state = !state;
    digitalWrite(led, state);
  }
  if (currentRotationTime!=rotationTime) {
    currentRotationTime = rotationTime;
  }
}

void magnetPresent() {
  timeNew = micros();
  rotationTime = timeNew - timeOld;
  timeOld = timeNew;
  pulse++;
}
