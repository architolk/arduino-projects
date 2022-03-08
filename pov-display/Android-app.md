# The Android app

The android app is based on the [simple bluetooth terminal](https://github.com/kai-morich/SimpleBluetoothTerminal) app. The changes are:

- An ImageView is included on the terminal page, including a button the select an image from the images located on the phone.
- After loading the image, the image is send as bytes to the ESP32 (the bluetooth server), displaying dots instead of the actual characters
- The "normal" terminal operation has been kept, so you can actually send and receive messages.

## BLE version

A BLE version of the simple bluetooth terminal app also exists: [SimpleBluetoothLeTerminal](https://github.com/kai-morich/SimpleBluetoothLeTerminal). This should make it fairly easy to change the communication from Bluetooth classic to Bluetooth Low Energy, making the Bluetooth connection less power consuming (I'm getting some power fluctuations in the LEDs I would like to avoid).

No specific profiles exist voor serial communication for BLE, but the most used is the one from Nordic, as this one is also used in [this arduino sketch](https://github.com/espressif/arduino-esp32/blob/master/libraries/BLE/examples/BLE_uart/BLE_uart.ino), which is actually part of the ESP32 examples for BLE. [This tutorial](https://randomnerdtutorials.com/esp32-bluetooth-low-energy-ble-arduino-ide/) gives some insights in the operation of BLE.
