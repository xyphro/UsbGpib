#!/usr/bin/env bash
# by D F Conway  v1.0  18 Sep 2024
# Linux version of a script to program the USB-GPIB adapter by Xyphro
#
# This shell file compiles the firmware then  programs Xyphro USB-GPIB adapter with the firmware in Linux.
# This batch file requires avr-gcc & avrdude to be installed.
# This batch file assumes that the .hex files are installed in a subdirectory (any name)
# avrdude is installed in.
#M To use this batch file:
# 1. Install avr-gcc & avrdude
# 2. Save this script in the directory  ~/ xxx /SW,
#    where for me xxx = ~/git/UsbGpib/SW
# 4. Change directory to run this script: ~/git/UsbGpib/SW/binaries
# 5. Connect the USB-GPIB adapter to a programmer and to a usb port on the PC with avrdude installed.
# 5. Run this script to execute the following commands.
echo "Compiles the firmware for USB-GPIB adapter"
echo "Then programs the adapter firmware"
echo "D Conway Oct 2024"
echo "###########################################"
cd ~/git/UsbGpib/SW/TestAndMeasurement
echo "Compiling the USB-GPIB firmware"
echo "###########################################"
make
cd ~/git/UsbGpib/SW/binaries
cp ~/git/UsbGpib/SW/TestAndMeasurement/TestAndMeasurement.hex .
#  This assumes that bootloader.hex is already in the binaries folder: ~/git/UsbGpib/SW/TestAndMeasurement/bootloader.hex
echo
echo "Programing the USB-GPIB adapter"
echo "###########################################"
avrdude -c usbasp -p m32u4 -e -Ulock:w:0x3F:m -Uefuse:w:0xcb:m -Uhfuse:w:0xd8:m -Ulfuse:w:0xff:m -U flash:w:TestAndMeasurement.hex -U flash:w:BootLoader.hex
# This avrdude command will install the firmware using a usbasp programmer.  
# Using a different programmer will require a different parameter.
echo
echo "DONE"
