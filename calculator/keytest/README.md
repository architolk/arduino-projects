# Keytest

The keytest routine checks if a key is pressed, and/or what configuration is currently active for the sliders

## Sliders
Sliders should not work on interupt bases, as they are active all the time

The routine is:
- Set pin 5, 6 and 7 LOW and as OUTPUT
- Set pin 1, 2, 3 and 4 as INPUT
- Set pin 5 HIGH
- Check which pin is also HIGH:
  - pin 1: + of + 3 2 1 0 F
  - pin 2: 2 of + 3 2 1 0 F (and pin 4 LOW)
  - pin 3: F of + 3 2 1 0 F
  - pin 4: 1 of + 3 2 1 0 F (and pin 2 LOW)
  - pin 4: 3 of + 3 2 1 0 F (and pin 2 HIGH)
  - if no pin is HIGH: 0
- Set pin 5 LOW and pin 6 HIGH
- Check which pin is also HIGH:
  - pin 1: 5/4 (or else ->)
  - pin 2: K (or else K not pressed)
  - pin 3: Paper feed (and pin 4 LOW)
  - pin 4: n+ (and pin 3 LOW)
  - pin 4: n+/- (and pin 3 HIGH) - or else: off
- Set pin 6 LOW and pin 7 HIGH
- Check which pin is also HIGH:
  - pin 1: print ON (or else: OFF)
  - pin 2: AM (or else: OFF)
- Repeat...

## Keys
Keys work on interupt:
- Set pin 8, 9, 10 and 11 LOW and as OUTPUT
- Set interupts on pin 11, 13, 15, 16, 20, 22, 24, 25 and as INPUT
- set global variable PinSelected = 0
- Set pin 8 HIGH
- Wait a while: interupt will set PinSelected to selected key (during this period, we can scan the sliders)
- set pin 8 LOW
- Check if PinSelected != 0, if so: key is pressed => Do something
- set PinSelected = 0
- set pin 9 HIGH
- etc...

## ESP32 pins

![](../../pov-display/esp32-d0wdq6-development-board-with-wifi-and-bluetooth-pinout-600x600.jpg)

Routing of the pins:

| ESP32 | Calc | Remark |
|-------|------|--------|
|GPIO36| Pin 11 | Interupt input |
|GPIO39| Pin 13 | Interupt input |
|RESET|
|GPIO34| Pin 15 | Interupt input |
|GPIO35| Pin 16 | Interupt input |
|GPIO32| Pin 20 | Interupt input |
|GPIO33| Pin 22 | Interupt input |
|GPIO25| - | Channel 1 audio |
|GPIO26| Pin 24 | Interupt input |
|GPIO27| Pin 25 | Interupt input |
|GPIO14| Pin 5 | Output |
|GPIO12| Pin 6 | Output |
|GND|
|5V|
|GPIO21| Pin 7 | Output |

| ESP32 | Calc | Remark |
|-------|------|--------|
|3V3|
|GPIO22| - | Internal LED |
|GPIO19| - | VSPI MISO |
|GPIO23| - | VSPI MOSI |
|GPIO18| - | VSPI SCK |
|GPIO5| - | VSPI SS |
|GPIO17| - | VFD Reset  |
|GPIO16| Pin 1 | Input |
|GPIO4| Pin 2 | Input
|GPIO0| Pin 8 | Output |
|GPIO2| Pin 3 | Input |
|GPIO15| Pin 9 | Output |
|GPIO13| Pin 10 | Output |
|GPIO1| Pin 11 | Output, TX pin |
|GPIO3| Pin 4 | Input, RX pin|

Merk op: alle pinnen zijn nodig, dus seriele communciatie werkt dan niet meer...

De 7 output pins krijgen een current limiting weerstand, zodat we zeker weten dat er niet te veel stroom gaat lopen tussen de output en input pins (10k moet kunnen volgens bepaalde sites, da's 0.33mA stroomverbruik - lekker weinig)
