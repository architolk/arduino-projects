# LED animation String

Operates on three strips of LED, each 150 LEDs

- 24 bits per LED
- 150 LEDs
- 3600 bits to be send
- 800kHz update frequency
- One frame per strip costs 4.5ms
- One frame for three strips costs 13.5ms
- 150 frames are needed to send a signal from start to finish
- Theoretically, this can be done in 0,675 seconds
- But... because we have three strips, it takes at least around 2 seconds (!)
