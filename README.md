# USB-GPIB Adapter 
## Version 2e (enhanced)

## Project Introduction

This project is a fork of the popular Xyphro USB-GPIB adapter.  The purpose of this version was to make some
enhancements to the circuit and PCB design.  It appears my design goals are slightly different to Xyphro. My aim was to enhance the design to make the adapter better value for money.  
I aimed to do this by making the adapter: 
1. easier to use,
2. easier to make and,
3. more resistant to ESD.

It was accepted that this would slightly increase the size and cost of the adapter.

The design goals were achieved with the following enhancements that:

1. fitted a USB-C connector,
2. added anti-static protection to the usb lines,
3. used both a red and green LED to enhance visual indications to the user,
4. added the option of surface mounted LEDs or through-hole LEDs on the same pcb,
5. added two holes in the pcb to anchor the pcb to the enclosure,
6. eased hand assembly.

I have been working on this project for quite a while and some of these features are already included in the Xyphro version 2 design.

## What you will find here

This repository includes the hardware and software files and information needed to build the USB-GPIB version 2e enhanced adapter.

## Acknowledgment

There are a number of USB-GPIB adapters out there but I consider the Xyphro design to be the best available and with the most potential for development.

It is fantastic that Xyphro has chosen to release his design for others to build and contribute to.

## Version Confusion

Xyphro has designed a version 1 and 2 adapter hardware.  I also have built two modified versions of the usb-gpib adapter.  To avoid confusion, I will refer to my latest design as version 2e because it is different to, and includes some enhancements not seen on, the Xyphro version 2 hardware design.

At the time of writing Xyphro has not written version 2 software to run on the version 2 hardware.  The version 1 software runs on the versions 1/2/2e hardware, but does not make use of the green LED.  The version 2e software available on this repository runs on versions 1/2/2e hardware.

## Sucessfully tested measurement equipment:
- R&S FSEM30
- R&S SMIQ03
- R&S CMU200
- R&S SMW200A
- R&S FSW
- R&S FSV
- Keithley 199 multimeter
- HP 3325A synthesizer/frequency generator
- HP 3457A multimeter
- HP 3478A multimeter
- Agilent E4406A VSA transmitter tester
- Tektronix TDS7104 Digital Phosphor Oscilloscope
- HP 8596A spectrum analyzer
- Agilent E3648A dual power supply

## Software Compatibility 

A key objective of this version 2e software  was to maintain full compatibility with the Xyphro firmware for versions 1,2 & 2e hardware.

This is my way of contributing back to the good work done by Xyphro and the other contributors.  The source code includes compiler directives that will produce version 2e firmware for either version 1 or 2/2e hardware.  Any future enhancements to the firmware source code will apply to versions 1/2/2e hardware.

# Hardware

## Hardware Compatibility

For similar reasons, the version 2e hardware design is fully compatible with the Xyphro design with one exception.
The Xyphro version 2 hardware includes a grounded MCU pin to enable software detection of the hardware version.  That feature is not used in my design because the code is compiled to either use a version 1 single LED adapter, or a version 2/2e red and green LED adapter.   There is then no requirement for the software to detect what version of hardware it is running on.

## Component sourcing

Most major suppliers will stock the parts required.  I used element14 and the BOM includes information needed to make an order.
For many parts, the Minimum Order Quantity (MOQ) is often greater than the number of parts required.  Often it is best to make a batch of say 5 or 10 adapters to minimise surplus parts.

The Centronic 24 pin connector can be sourced from Aliexpress.  These are low cost and the quality is quite reasonable.

## PCB

I used KiCAD to draw the circuit and design the PCB.  All the files are included here in the HW folder if you want to make further modifications.  The Gerber files are also here if you simply want to have PCBs made.

## Assembly

I just use a hand soldering iron to assemble the parts to the PCB. The parts were selected to ease hand assembly.  Start with pre-tinning the USB-C connector and MCU pins.  I fitted the MCU, then the USB-C connector.  I found that the MCU failed if a heat gun was used, most likely because of my bad technique, but using a soldering iron worked well.  Solder the remaining smd parts and the LEDs.  I then beeped all of the USB-C connector pins and the MCU pins looking for solder bridges and bad joints.  The adapter can be tested by programming it.  If the red LED flashes, the adapter is OK.  Lastly I soldered in the Centronic connector. 

## Enclosure TODO

Often significant force is required to connect and separate the adapter from the instrument GPIB connector.  For this reason it is highly recommended that the pcb is fitted within an enclosure.  The enclosure is a custom two piece design.  FreeCAD was used to create the design.  The enclosure is printed with any good 3D printer. It requires 2x M3 screws to connect the PCB to the enclosure.

# Software

## Source code

The source code of the Boot loader (slightly modified LUFA MassStorage Boot loader) and the main USB-GPIB code are located in the "SW" subdirectory.
At the time of publication LUFA 170418 release was used, with avr-gcc as compiler.  The hex or bin files are loaded onto the adapter using avr-dude.  

The Software is compatible with version 1, 2 and 2e hardware.  Compiling for version 1 hardware requires the compiler directive VER2 to be commented out.  Compiling for versions 2/2e requires the VER2 compiler directive to be active. The core functionality is identical for all versions.  Any future improvements made to the source code will be available for versions 1/2/2e of the hardware.

The only effect of running a software version that doesn't match the hardware version is that the LEDs won't light up as expected.  

## Binaries

For those, that just want to create their own device, I've included the binary output in the "SW/binaries" subdirectory.  The binaries are compiled for versions 2/2e of the hardware.

## Bootloader

The boot loader allows the firmware to be loaded into the adapter via the USB connection. I have never used this feature.
Activating this mode requires shorting of two pins (tweezers).   The pins are marked on the ver 2e PCB silk screen for the ISP connector.

## Loading the Firmware with USB-ASP Programmer

I use a cheap usb-avr programmer from Aliexpress and avr-dude.  Search for "avr usbasp programmer".  
You will need the:
1.  programmer,
2.  flat 10-way cable assembly and,
3.  6pin to 10pin adapter

The /SW/ directory contains a windows batch file that runs the command that loads the firmware.

There is also a Linux script that compiles the code and then loads the firmware onto the adapter via the programmer.

If they don't work for you, check that the directories in the scripts match your own directory structure. Try running the commands from the command line interface.

# Using the device

## USB enumeration

You might be surprised initially, that the device does not show up in your device manager (or lsusb), when you connect only the USB side. This is a feature, not a bug (really!).
Only, if a GPIB device is connected, you can see the device on your PC too.

The reason behind the feature is simple: Instead of having a standard GPIB wiring, where you have a single GPIB controller and lots of GPIB devices interconnected, USBGPIB supports only a direct connection of the USBGPIB device to your measurement device. If you have like me e.g. 14 Instruments you don't want all to show up in the device manager, if the measurement device itself is powered down - you won't anyway be able to communicate with a powered down device.

When USB and the GPIB side is connected, the device enumerates. The USBGPIB device reads out the ID of the instrument and constructs a unique USB Serial number out of it. It is thus easily possible to associate multiple connected USBGPIB devices with the measurement instrument.

The VISA resource name is constructed from this USB Serial number. You can identify easily e.g. in NiMax, which device is connected:

<img src="https://raw.githubusercontent.com/xyphro/UsbGpib/master/pictures/NiMaxExample.png" width="90%"/>

If you connect your USBGPIB device afterwards to another GPIB measurement device, it will disconnect and connect with a new serial number string, matching the other GPIB device *IDN? response again.

## Scenarious

- The adapter has been tested with operating systems Windows 7 and 10 so far only. But linux should also work out of the box.
- USB1.1, USB2.0 and USB3.x ports tested, with and without USB HUB in between.
- The connection stays responsive, when power cycling the PC, or hibernating/sleeping it
- Different connection cycles (GPIB side connected first, USB side connected first, swapping GPIB side equipment, ...)
- Extensive testing of timeout scenarios. E.g. making an illegal query and testing, if the USBTMC handles the timeouts properly. This was a very tricky part to get right.
- Tested special transfer modes. E.g. capturing screenshots from different instruments is usually something, which will drive other GPIB adapters to the limits, because binary data of unknown length needs to be transported successfully.


## GPIB settings on your measurement device

GPIBUSB does probe all GPIB primary addresses (and secondary address 0) for presence of a GPIB Talker/listener. It is thus not required to set a specific GPIB address - GPIBUSB will find it itself.

The only importance setting on the measurement device is, that the GPIB interface is enabled, which is typically the case.

## LED indicator

The LED indicates different states.  Version 1 hardware only has a red LED.  Version 2/2e hardware has a red and green LED. Version 2e software can be compiled to run correctly on version 1 or 2/2e hardware.

### Version 1 : Red LED Only
LED blinking: The USBGPIB converter is connected to a measurement instrument, it is powered off or its GPIB port is disabled. In this state, the device is also not connected to USB and will not show up in the device manager or lsusb.
LED on: The device is connected to a measurement device and GPIB communication possible. It is also accessible over USB
LED off: The device is not connected over USB, or the PC powered off :-)

### Version 2/2e : Red & Green LED
RED LED blinking : The USB-GPIB adapter is connected via USB, but the gpib port is not enabled.
RED LED off: The device is not connected to USB, or the PC powered off.
GREEN LED on: The USB-GPIB adapter is connected to a powered-up measurement device.
GREEN LED pulsing: Active GPIB communication with the measurement device.

The pulsing green LED indicates activity, not quantity of messages.
The green and red LEDs are never on at the same time.

# Setting Parameters

The firmware features human readable text base configuration of several parameters.

The command parser is quite simple. For that reason follow the exact syntax as shown below. 
Don't add extra spaces or make other modifications or concatenate commands.

## Read termination method

While most GPIB interfaces use the hardware signal EOI to signal the end of a message, not all old equipment supports it. Some older instruments even don't have the EOI pin hardware wise wired and use \r or \n termination.

The following commands are available:

### Set read termination to CR (\r):
```
dev.control_in(0xa1, 0x40, 0, 0, 1); # USBTMC pulse indicator request (enables internal command processing)
dev.write('!term cr')
```
Above setting is volatile. To make this a permanent setting call the below mentioned "!term store" command.

### Set read termination to LF (\n):
```
dev.control_in(0xa1, 0x40, 0, 0, 1); # USBTMC pulse indicator request (enables internal command processing)
dev.write('!term lf')
```
Above setting is volatile. To make this a permanent setting call the below mentioned "!term store" command.

### Set read termination to EOI only (default setting):
```
dev.control_in(0xa1, 0x40, 0, 0, 1); # USBTMC pulse indicator request (enables internal command processing)
dev.write('!term eoi')
```
Above setting is volatile. To make this a permanent setting call the below mentioned "!term store" command.

### Save readtermination setting in eeprom (make them non-volatile)
```
dev.control_in(0xa1, 0x40, 0, 0, 1); # USBTMC pulse indicator request (enables internal command processing)
dev.write('!term store')
```

### Query current termination setting from device
```
dev.control_in(0xa1, 0x40, 0, 0, 1); # USBTMC pulse indicator request (enables internal command processing)
print(dev.query('!term?'))
```
This returns a text string containing "lf", "cr" or "eoi"


## AutoID setting

Default wise the GPIB adapter tries during power on of the instrument to query using *IDN? or ID? commands the instrument name automatically.
This is used to build the USB serial number, which finally gets part of the VISA resource string.

Not all instruments support this *IDN / ID? query. For this reason this feature can be turned off.
The serial number will then be built based on the GPIB address of the instrument.

### Turn AutoID feature off
```
dev.control_in(0xa1, 0x40, 0, 0, 1); # USBTMC pulse indicator request (enables internal command processing)
dev.write('!autoid off')
```
This setting is stored in eeprom = non volatile memory, so will survive a power cycle

### turn AutoID feature on
```
dev.control_in(0xa1, 0x40, 0, 0, 1); # USBTMC pulse indicator request (enables internal command processing)
dev.write('!autoid on')
```
This setting is stored in eeprom = non volatile memory, so will survive a power cycle

### turn auto ID on with delay

Some instruments need after turn on some seconds before GPIB is responsive.

With below 3 settings you can set a delay which is applied before the instrument ID is queried after power on.
Note that it will take then also more time, before the USB device is recognized by the PC.

Also this setting is non-volatile.

Delay 5 seconds:
```
dev.control_in(0xa1, 0x40, 0, 0, 1); # USBTMC pulse indicator request (enables internal command processing)
dev.write('!autoid slow')
```

Delay 15 seconds:
```
dev.control_in(0xa1, 0x40, 0, 0, 1); # USBTMC pulse indicator request (enables internal command processing)
dev.write('!autoid slower')
```

Delay 30 seconds:
```
dev.control_in(0xa1, 0x40, 0, 0, 1); # USBTMC pulse indicator request (enables internal command processing)
dev.write('!autoid slowest')
```

### read autoid setting

```
dev.control_in(0xa1, 0x40, 0, 0, 1); # 
print(dev.query('!autoid?'))
```
Returns as text string either: "off", "on", "slow", "slower" or "slowest".

## Firmware version

Finally I implemented a command to query the USB adapters firmware version :-)

```
dev.control_in(0xa1, 0x40, 0, 0, 1); # USBTMC pulse indicator request (enables internal command processing)
print(dev.query('!ver?'))
```

## Shorten resource strings (Matlab)

A user discovered that Matlab has a limitation in the VISA resource string length and shared a pull request to reduce the length.
I expose this now first time in the baseline firmware with the following options.

This setting is stored in eeprom = non volatile.

### Limit the USB serial number to a length of 20 characters
```
dev.control_in(0xa1, 0x40, 0, 0, 1); # USBTMC pulse indicator request (enables internal command processing)
dev.write('!string short')
```

### disable limitation of USB serial number length (default behavior)
```
dev.control_in(0xa1, 0x40, 0, 0, 1); # USBTMC pulse indicator request (enables internal command processing)
dev.write('!string normal')
```

### query the string length setting.
```
dev.control_in(0xa1, 0x40, 0, 0, 1); # USBTMC pulse indicator request (enables internal command processing)
print(dev.query('!string?'))
```

This returns as text string either "normal" or "short".

## reset the adapter
```
dev.control_in(0xa1, 0x40, 0, 0, 1); 
dev.write('!reset')
```

Do a reset of the adapter. Note that due to the reset you have to close the visa session and start a new one.



# What's Next ???

I am presently working on a 3D enclosure design that will have more curves than straight lines.  The design will be consistent with my view that good engineering looks good.
I will also upload the hardware design and pcb files.  People can then choose between the different flavours of adapter hardware designs to suit their needs.  

