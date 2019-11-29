# Audio player

## Ports

0..7: PORTD: 8 bits DAC digital output
8..9, A0, A1, A2, A3: LED outputs
A4: Button inputs (via resistors analog inputs)
10..13: SD card controller over SPI

## Refs

SD en Audio DAC:
- https://www.instructables.com/id/Arduino-playing-wav-files/

LM386 beschrijvingen:
- https://www.youtube.com/watch?v=4ObzEft2R_g
- https://www.youtube.com/watch?v=P4GsoMTv-SY

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

Zie: https://electronics.stackexchange.com/questions/269702/why-does-an-analog-chip-have-two-ground-connections-but-not-two-vcc-connections

De DAC kan op basis van REFA een ander voltage krijgen dan de digitale poorten
De RFBA betreft een feedback loop, die is voor connectie met een OpAmp

We zouden bijvoorbeeld:
- AGnd als virtual Ground op 4,5 volt
- REFA als voltage op 7,5 Volt (je krijgt dan 3 Volt uitslag)
- RFBA hoeven we voorlopig niet te gebruiken
- OUTA geeft dan een uitslag van -3 naar -7 Volt, de OpAmp moet dan in inverted mode worden gebruikt
- De TC072 OpAmp krijgt dan als V- 0 Volt en V+ 9 Volt
- Resultaat van de gebufferde output zou dan +3 naar +7 volt zijn

Handiger zou een LM324 of LM358 zijn, die werkt rail-to-rail, en wordt ook in voorbeelden gebruikt...
Bv in: https://www.chegg.com/homework-help/questions-and-answers/1-determine-values-r1-r2-circuit-get-output-range-form-5v-5v-binary-input-digital-value-go-q26752470

https://nl.banggood.com/DAC0832-DIY-Module-Kits-DAC-Waveform-Generator-Unipolar-Bipolar-Outputs-Electronic-Component-p-1353417.html

(DAC0832 is een andere dan de TLC7528, maar ze lijken wel sterk op elkaar)

|Pin|Function|
|---|--------|
| 1 | AGND (Analog Ground) |
| 2 | OUTA (Output A) |
| 3 | RFBA (Feedback A) |
| 4 | REFA (Reference A) |
| 5 | DGND (Digital Ground) |
| 6 | DACA/DACB (Switch) |
| 7 | DB7 (MSB) |
| 8 | DB6 |
| 9 | DB5 |
|10 | DB4 |
|11 | DB3 |
|12 | DB2 |
|13 | DB1 |
|14 | DB0 (LSB) |
|15 | CS |
|16 | WR |
|17 | VDD (Digital Voltage source) |
|18 | REFB (Reference B) |
|19 | RFBB (Feedback B) |
|20 | OUTB (Output B) |

Current setup (from instructable):

|DAC Pin|Arduino Pin|Function|
|-------|-----------|--------|
| 1 | GND | AGND (Analog Ground) |
| 2 | +5V | OUTA (Output A) |
| 3 | +5V | RFBA (Feedback A) |
| 4 |     | REFA (Reference A) |
| 5 | GND | DGND (Digital Ground) |
| 6 | GND | DACA/DACB (Switch) |
| 7 |   7 | DB7 (MSB) |
| 8 |   6 | DB6 |
| 9 |   5 | DB5 |
|10 |   4 | DB4 |
|11 |   3 | DB3 |
|12 |   2 | DB2 |
|13 |   1 | DB1 |
|14 |   0 | DB0 (LSB) |
|15 | GND | CS |
|16 | GND | WR |
|17 | +5V | VDD (Digital Voltage source) |
|18 |     | REFB (Reference B) |
|19 | +5V | RFBB (Feedback B) |
|20 | +5V | OUTB (Output B) |

(Pin 6, 15 and 16 were originally connected to pin 8, 9 and 10 of the arduino, but as we don't use them, we can set them to digital LOW = GND without any problems).

Something is not quite right in this case: RFBA and RFBB should not be +5V, but are designed for the feedback of the OpAmp.

And as we don't use channel B, we can simply ground OUTB and not bother with RFBB.

A better configuration (also works, haven't checked output of DAC on scope yet):

|DAC Pin|Arduino Pin|Function|
|-------|-----------|--------|
| 1 | GND | AGND (Analog Ground) |
| 2 | +5V | OUTA (Output A) |
| 3 |     | RFBA (Feedback A) |
| 4 |     | REFA (Reference A) |
| 5 | GND | DGND (Digital Ground) |
| 6 | GND | DACA/DACB (Switch) |
| 7 |   7 | DB7 (MSB) |
| 8 |   6 | DB6 |
| 9 |   5 | DB5 |
|10 |   4 | DB4 |
|11 |   3 | DB3 |
|12 |   2 | DB2 |
|13 |   1 | DB1 |
|14 |   0 | DB0 (LSB) |
|15 | GND | CS |
|16 | GND | WR |
|17 | +5V | VDD (Digital Voltage source) |
|18 |     | REFB (Reference B) |
|19 |     | RFBB (Feedback B) |
|20 | GND | OUTB (Output B) |

Even better would be to use the original spec with an OpAmp. But I only have some TL072 around...

The current operation of the DAC is at 0-5V, but line level is actually +/- 1,5V, so 0-3V would be better:

TL072 Pin configuration:

|Pin|Function|
|---|--------|
| 1 | Output A |
| 2 | Inverted input A (-) |
| 3 | Non-inverted input A (+) |
| 4 | V- |
| 5 | Non-inverted input B (+) |
| 6 | Inverted input B (-) |
| 7 | Ouput B |
| 8 | V+ |

Configuration:

|Pin| Routing |
|---|---------|
| 1 | Pin 3 of the DCA (RFBA) and output to speaker/amplifier |
| 2 | Pin 2 of the DAC (OUTA) |
| 3 | Virtual ground (AGND), +4.5V |
| 4 | GND, 0V |
| 5 | Not used |
| 6 | Not used |
| 7 | Not used |
| 8 | +9V |

The AGND is the ground for the whole speaker/amplifier part, so the speaker should be connected to the virtual ground, NOT the actual battery ground!

And for the DAC:

|DAC Pin|Arduino Pin|Function|
|-------|-----------|--------|
| 1 | +4.5V | AGND (Analog Virtual Ground) |
| 2 | OA2 | OUTA (Output A), OpAmp pin 2 |
| 3 | OA1 | RFBA (Feedback A), OpAmp pin 1 |
| 4 | +7.5V | REFA (Reference A) |
| 5 | GND | DGND (Digital Ground) |
| 6 | GND | DACA/DACB (Switch) |
| 7 |   7 | DB7 (MSB) |
| 8 |   6 | DB6 |
| 9 |   5 | DB5 |
|10 |   4 | DB4 |
|11 |   3 | DB3 |
|12 |   2 | DB2 |
|13 |   1 | DB1 |
|14 |   0 | DB0 (LSB) |
|15 | GND | CS |
|16 | GND | WR |
|17 | +5V | VDD (Digital Voltage source) |
|18 |     | REFB (Reference B) |
|19 |     | RFBB (Feedback B) |
|20 | GND | OUTB (Output B) |

Virtual ground:
- Resistor devider with 4.7K resistors
- 470uF capacitor for stability

+9V - 4K7 - +4.5V - 4K7 - 0V (Capacitor: +4.5V - u470F - 0V )

Kinda the same circuit for the +7.5V reference voltage:
- Resistors should have a 1.2 quotient, for example 4.7K resistors, with 2 470 resistors at one end:

+9V - 470R - 470R - +7.5V - 4K7 - 0V (Capacitor: +7.5V - u470F - 0V)

Dual voltage:
|Pin| Routing |
|---|---------|
| 1 | Pin 3 of the DCA (RFBA) and output to speaker/amplifier |
| 2 | Pin 2 of the DAC (OUTA) |
| 3 | GND, 0V |
| 4 | -9V |
| 5 | Not used |
| 6 | Not used |
| 7 | Not used |
| 8 | +9V |



For the SD card:

|SD Card|Arduino|
|-------|-------|
| GND   | GND   |
| +3.3V |       |
| +5V   | +5V   |
| CS    | 10    |
| MOSI  | 11    |
| SCK   | 13    |
| MISO  | 12    |
| GND   |       |
