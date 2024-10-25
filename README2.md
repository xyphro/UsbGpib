# USB-GPIB Adapter : Version 2 Enhanced

## Project Introduction

This project is a fork of the popular Xyphro USB-GPIB adapter.  The purpose of this version was to make some
enhancements to the circuit and PCB design.  It appears my design goals are slightly different to Xyphro. My aim was to make enhancements to make the adapter better value for money.  I aimed to do this by making the adapter: 
1. easier to use,
2. easier to make and,
3. more resistant to normal operation.

It was accepted that this would slightly increase the size and cost of the adapter.

The design goals were achieved when the following enhancements were included:

1. a USB-C connector,
2. adding anti-static protection to the usb lines,
3. using both a red and green LED to enhance visual indication to the user,
4. adding the option of surface mounted LEDs or through-hole LEDs,
5. add two holes in the pcb to anchor the pcb to the enclosure,
6. ease of hand assembly.

I have been working on this project for quite a while and some of these features are already included in the Xyphro version 2 design.


## Acknowledgment

There are a number of USB-GPIB adapters out there but I consider the Xyphro design to be the best available and with the most potential for development.

It is fantastic that Xyphro has chosen to release his design for others to build and contribute to.

## Software Compatibilty 

A key objective of this version 2 software  was to maintain full compatibility with the Xyphro firmware for both verions 1 & 2 hardware.

This is my way of contributing back to the good work done by Xyphro and the other contributors.  The source code includes compiler directives that will produce version 2 firmware for either version 1 or 2 hardware.  Any future enhancements to the firmware source code will apply to both version 1 and 2 hardware.

## Hardware Compatibility

For similar reasons, the version 2 hardware design is fully compatible with the Xyphro design with one exception.  
The Xyphro version 2 hardware includes a grounded MCU pin to enable software detection of the hardware version.  

## What's Next ???

I am presently working on a 3D enclosure design that will have more curves than straight lines.  The design will be consistent with my view that good engineering looks good.
I will also upload the hardware design and pcb files.  People can then choose between the different flavours of adapter hardware designs to suit their needs.  

