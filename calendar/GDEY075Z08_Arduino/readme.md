# GDEY075Z08_Arduino

This test program is a copy from the [Good-display example code for the GDEY075Z08](https://www.good-display.com/companyfile/1391.html).

Two changes where made:
- setting the correct pin numbers for the ESP32 in GDEY075Z08_Arduino.ino and Display_EPD_W21_spi.h
- Adding `digitalWrite(13,HIGH);` at the beginning of GDEY075Z08_Arduino.ino (setting the PWR pin to high)
