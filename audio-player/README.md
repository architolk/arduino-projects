# Audio player

## Ports

0..7: PORTD: 8 bits DAC digital output
8..9, A0, A1, A2, A3: LED outputs
A4: Button inputs (via resistors analog inputs)
10..13: SD card controller over SPI

## Refs


TL072 beschrijving (de buffer):
https://www.instructables.com/id/Arduino-Audio-Input/

tlc7528cn beschrijving (de DAC):
https://www.instructables.com/id/Stereo-Audio-with-Arduino/

https://www.avrfreaks.net/forum/r2r-dac-op-amp-buffer-advice

Dit geeft m'n probleem goed weer. Feitelijk is de TL072 niet zo geschikt. Rail-to-rail OpAmps are needed...


http://www.circuitbasics.com/build-a-great-sounding-audio-amplifier-with-bass-boost-from-the-lm386/
Hier in mooie circuits voor de LM386

FET (en BJT's) voor buffer:
https://www.electronics-notes.com/articles/analogue_circuits/fet-field-effect-transistor/common-drain-source-follower-circuit.php
http://www.ciphersbyritter.com/RADELECT/PREJFET/JFETPRE.HTM
http://www.muzique.com/lab/buffers.htm

Deze werkt ook met transistors, en zelfs geen amp:
https://www.youtube.com/watch?v=tUapZ_JdHLE


Het lijkt er dus op dat ik het voor elkaar kan krijgen met:
1. Extra BJT transistor als buffer tussen de R2R ladder en de uitgang;
2. Een LM386 als audio amplifier

Probleem is dat de voltage devider de R2R ladder stoort. Oplossing is gewoon direct met de BJT verbinden, zonder capacitor. Probleem is dan wel dat ie niet netjes rond 0 volt centreert, en aan de onderkant niet helemaal netjes is.

Beter is echt wel een rail-to-rail OpAmp gebruiken, bijvoorbeeld (LM358):
https://forum.arduino.cc/index.php?topic=494646.0
(single supply OpAmp)
