# rc-firmware-hifive1

This project includes device drivers for the Hifive1 Rev B board and its FE310-G002 SoC as well as some sample applications. Eventually I'm hoping to use this board for testing some RC vehicles. Everything is work-in-progress and I'm still experimenting with the board.

## Sample Applications

The application that uploaded to the hardware is controlled by the macro defined in `src/main.cpp`. There are currently 3 applications:

### HELLO_APP
This is the SiFive "Hello World" application

### WIFI_APP
This is a demo that interfaces with the onboard ESP32 via SPI. It was started from an example that someone else based on SiFive's driver for Zephyr RTOS. Currently the link with the ESP32 is a bit unstable but that should be fixed as I phase out components of the original app and replace them with custom drivers.

### ESP32_AT_APP
I created this to work from the ground up for communicating with the ESP32 but ended up not doing anything with it. It is practically empty and also out of date.

## Unit Testing
There are some unit tests which are intended to run on a native desktop platform. Using the PIO CLI, run `pio test -e native`.
