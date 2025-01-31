# UsbGpib
Versatile, cheap, portable and robust USB to GPIB converter (USBTMC class based).

You'll find many projects like this, but this one is special (ok, everybody will claim this) :-)

<img src="https://raw.githubusercontent.com/xyphro/UsbGpib/master/pictures/UsbGPIB.jpg" width="80%"/>

If you have a lot of test equipment at home, you might know the issues: Lots of devices only have a GPIB interface and the GPIB adapters and GPIB cables on the market are very expensive and some of them even have many issues when run under Windows 10 (device driver does not work). Or they e.g. are not able to be operated with VISA, because they are UART based, need special command sequences, ...

<img src="https://raw.githubusercontent.com/xyphro/UsbGpib/master/pictures/SizeComparison.jpg" width="40%" align="right"/>
The adapters are also typically very long, such that they extend the overall length of your test equipment by at least 10cm (~4 inches).

Besides the 2 very big manufacturers, other GPIB adapters, e.g. with Ethernet or USB interfaces are not recognized by normal VISA providers or PyVisa, making the measurement control implementation specific for your GPIB adapter.

I got frustrated and tried to turn that frustration into something positive. Here's a video showing the final device in action - click to view:

[![](https://img.youtube.com/vi/pZp1QZCXrF8/0.jpg)](https://www.youtube.com/watch?v=pZp1QZCXrF8) 


Some goals of the project were:
- Work based on the standard USBTMC protocol. This allows the GPIB test equipment to look like a normal USB based measurement device and work flawless with e.g. NI VISA, Labview, Matlab or PyVisa.
- Have a small length - otherwise my eqipment risks falling off the shelf :-) Also the USB cable should connect at an angle of 90 degrees to make it shorter.
- It should be cheap but still versatile (you can build a single one of these for only 14 USD!)
- It should support ALL my test equipment, from many different GPIB implementation generations and different GPIB flavors
- The Firmware should be upgradeable over USB
- It should be rock-solid (!) I don't want to end up in a very long measurement being interrupted because of a software issue in my USB GPIB converter.
- It should support additional features like serial poll and remote enabling/disabling
- If there is no GPIB device connected to the USBGpib converter, or the GPIB device is powered down, there should be no USB device visible on the PC.

All those goals are met.

# 12th January '25 update

## New GUI available!

I implemented a small GUI to change the non-volatile settings of the GpibUSB adapter. Sorry - it's windows only :-)
It is a single .exe file without external dependencies. You can just download it, run it and immediately change settings.

<img src="https://raw.githubusercontent.com/xyphro/UsbGpib/master/pictures/UsbGpibGUI.png" width="80%"/>

Please download it from this folder: [SW/UsbGpibGUI](SW/UsbGpibGUI)

The source code is included in the `src` subdirectory.

## Python GUI

UsbGpib user Jim Houston created an impressive pure Python GUI similar to mine - and I believe he began working on it even before I released mine. His GUI stands out for its versatility and platform independence, requiring only Python to run. It can be executed directly with Python 3 and should run on most platforms.

For details about it, have a look into this discussion thread: [https://github.com/xyphro/UsbGpib/discussions/55](https://github.com/xyphro/UsbGpib/discussions/55)

The GUI can be found here: [SW/AlternativeUsbGpibGui](SW/AlternativeUsbGpibGui)

## New Firmware

I release also herewith Version 1.8 of the firmware. 
Mainly a bugfix was done: the string setting now also returns short, when it is actually set to short. (issue report #59).

# 13th January '24 update

New year, new update :-)

I realized a user suggestion: a better human readable interface to the internal instrument settings.
This also exposes new functionality, e.g. shortening of the VISA resource string, adjustable delayed application of AutoID features, ...
The read termination setting can now be set in a volatile way and also be read back.

A slight issue snuck in also and got fixed: After an instrument clear the next GPIB transfer timed out. Nobody found it so far though :-) But I fixed it.

Have fun using this!
(would anybody volunteer to make a simple cross platform gui for the non volatile settings :-) ?)

# 19th December '23 update

Finally I made it - the new HW design is on this page. As I had to restructure the repository a bit it took quite a while.
The software image is always the same and runs on both HW revisions.

<img src="https://raw.githubusercontent.com/xyphro/UsbGpib/master/pictures/Upcoming_Rev2.png" width="40%"/>

I also have put a binary for a new Firmware image in there. I am still busy optimizing certain things but want to give you a try before doing a next release.
Please if you test it: Feed it back to me - does it work / does it break anything -> sharing is caring. 

Mail me at `xyphro@gmail.com` or raise an issue under the issue section.

[FW image location: SW/binaries](SW/binaries)
The file `TestAndMeasurementReleaseCandidate.bin` is the new improved firmware. The `TestAndMeasurement.bin` is the older stable firmware.

Changes in this release candidate are as announced previously:
- 488.2 support further rolled out. This means Status bytes are automatically read out and reported via interrupt transfers to PC as foreseen in the standard.
- FASTER (!) 7x write speed improvement and 4x times read speed improvement. As reference: On an FSW I get 310kbytes/s as write transfer speed and 240kBytes/s as read transfer speed.
- A fix for read status byte readout leading to issues on my CMU200
- Several smaller race conditions identified and fixed. A lot of work went into stress testing 488.2.

As said, I feel this is the best Firmware image so far, but I changed so much that I want to go first for this small betatest phase asking you explicitly for positive and negative feedback.

After further testing I will also push the source code of this image.

# Hardware

## Microcontroller choice

Although nowadays I typically would prefer an ARM Cortex M0/3/4/7 controller, there is an issue with it. Most available Cortex-M devices support only max. 3.3V supply voltages, such that there would be a requirement for a level shifter towards the GPIB Bus.
GPIB is based on 5V (not exactly true, but a first approximation).

This limited the microcontroller choice to e.g. AVR or PIC controllers. Because of very good availability I ended up with an ATMEGA32U4 microcontroller.
In addition to the device supporting 5V I/O voltage, it also does not require a regulator to be part of the application, as it has an internal 3.3V regulator. This minimizes the full application schematic and BOM.

Apart from that, there is an excellent USB stack available [http://www.fourwalledcubicle.com/LUFA.php](http://www.fourwalledcubicle.com/LUFA.php).

The GPIB side of the schematic can be directly connected to the ATMega32U4 IO pins. The IO pins from the microcontroller side are only set to 2 different states: Tristate (input) or output LOW, to talk over GPIB.

## Component sourcing

All components are easy to source, so I only specify the potentially critical ones:

- 16 MHz Crystal: Farnell 2853867 - MCSJK-7E-16.00-8-30-100-B-30 
- REV 1 GPIB connector: Farnell 2421095 - NORCOMP 112-024-113R001. For REV 2 use a straight 24P male solder type connector e.g. from AliExpress.
- USB connector: Farnell 2668483 - Amphenol ICC 61729-1011BLF

## PCB

The PCB can be ordered at nearly any PCB pool production service (e.g. 10 PCBs for 2 USD + shipping). The Gerber files are included in the "HW/Gerber files" subdirectory.

## Mounting the PCB

The PCB is available in 2 revisions.
- [REV 1](HW/REV1/README.md) is the most widely used right now due to age. It has a USB Type-B connector and an L-shaped housing visible on a few photos of this page.
- [REV 2](HW/REV2/README.md) has some improvements like being smaller, fitting better and having a USB Type-C connector.

Choose whatever you prefer. The software images and also the external behavior is the same.

# Housings

## REV 1
<img src="https://raw.githubusercontent.com/xyphro/UsbGpib/master/pictures/housing.png" width="33%"/><img src="https://raw.githubusercontent.com/xyphro/UsbGpib/master/pictures/housing_snap.png" width="50%"/>

I created a sophisticated 3D printable housing for this adapter. The design was made with Fusion 360. The project file and the STL files are included in the `Housing` subdirectory.

The PCB fits perfectly into it. Optionally it can be fixed with 2 mounting screws (the GPIB connector has 2 threads, use 2 times 4-40 UNC x 3/8) and the TOP cover snaps onto the housing base.

I printed this using an Ender 5 3D printer with black PLA, 0.15mm layer height, 1mm wall thickness, no support.
Take care, that you rotate the TOP part of the housing by 180 degrees, so that the flat side is located on the printer bed.
Printing works fine, several iterations of the design were made to ensure good printability.
I printed so far 15 housings, without a single fail.

More information on this REV 1 can be found here: [REV 1](HW/REV1/README.md).
Note that the programming/build instructions also moved to this location.

## REV 2

<img src="https://raw.githubusercontent.com/xyphro/UsbGpib/master/pictures/Upcoming_Rev2.png" width="40%"/>

The REV 2 housing is a lot smaller, but requires 2 screws.
The housing is quite important to be able to connect and disconnect the board without breaking anything. It is key for mechanical stability of the adapter.
When operating the device without a housing, be very careful when plugging in and out the board in case the GPIB connector fits very tightly.

Information on how to build it can be found in the HW/REV 2 folder: [REV 2](HW/REV2/README.md).
Note that the programming/build instructions also have moved to this location.

# Software

## Source code

The source code of the Boot loader (slightly modified LUFA MassStorage Boot loader) and the main USBGPIB converter are located in the `SW` subdirectory.
At the time of publication LUFA 170418 release was used, with GCC as compiler.

Note: the Software is compatible with any HW revision in this repository. For REV 1 and REV 2 hardware you don't need different SW images.

## Binaries

For those of you who just want to create their own device I've included the pre-built binary output in the `SW/binaries` subdirectory.

# Using the device

## USB enumeration

You might be surprised initially, that the device does not show up in your device manager (or `lsusb`), when you connect only the USB side. This is a feature, not a bug (really!).
Only if a GPIB device is connected can you see the device on your PC too.

The reason behind the feature is simple: Instead of having a standard GPIB wiring, where you have a single GPIB controller and lots of GPIB devices interconnected, USBGPIB supports only a direct connection of the USBGPIB device to your measurement device. If you have like me e.g. 14 Instruments you don't want all to show up in the device manager, if the measurement device itself is powered down - you won't be able to communicate with a powered down device anyway.

When USB and the GPIB side is connected, the device enumerates. The USBGPIB device reads out the ID of the instrument and constructs a unique USB Serial number out of it. It is thus easily possible to associate multiple connected USBGPIB devices with the measurement instrument.

The VISA resource name is constructed from this USB Serial number. You can identify easily e.g. in NiMax which device is connected:

<img src="https://raw.githubusercontent.com/xyphro/UsbGpib/master/pictures/NiMaxExample.png" width="90%"/>

If you connect your USBGPIB device afterwards to another GPIB measurement device, it will disconnect and connect with a new serial number string, matching the other GPIB device `*IDN?` response again.

## GPIB settings on your measurement device

GPIBUSB does probe all GPIB primary addresses (and secondary address 0) for presence of a GPIB Talker/listener. It is thus not required to set a specific GPIB address - GPIBUSB will find it itself.

The only importance setting on the measurement device is, that the GPIB interface is enabled, which is typically the case.

## LED indicator

The LED indicates different states:

### LED blinking:
The USBGPIB converter is disconnected or connected to a measurement instrument that is powered off or has its GPIB port disabled. In this state, the device is also not connected to USB and will not show up in the device manager or `lsusb`.
### LED on:
The device is connected to a measurement device and GPIB communication possible. It is also accessible over USB.
### LED off:
The device is not connected over USB, or the PC powered off :-)

## Controlling GPIB devices
<img src="https://raw.githubusercontent.com/xyphro/UsbGpib/master/pictures/towerOfGpib.jpg" width="30%" align="right"/>


As this converter implements the standard USBTMC Test and measurement class, you can control your instrument from ANY of the normal VISA tools. I tried so far R&S VISA and NI Visa, using Python and Matlab to talk to the devices.

# Testing status

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
- Agilent E4406A VSA transmitter tester
- Tektronix TDS7104 Digital Phosphor Oscilloscope
- HP 8596A spectrum analyzer
- Agilent E3648A dual power supply

## Scenarios

- I tested under Windows 7 and 10 operating systems so far only. But Linux and Mac OS should also work out of the box.
- USB1.1, USB2.0 and USB3.x ports tested, with and without a USB hub in between.
- The connection stays responsive when power cycling, hibernating, or sleeping the PC.
- Different connection cycles (GPIB side connected first, USB side connected first, swapping GPIB side equipment, ...)
- Extensive testing of timeout scenarios. E.g. making an illegal query and testing whether the USBTMC handles the timeouts properly. This was a very tricky part to get right.
- Tested special transfer modes. E.g. capturing screenshots from different equipment is usually something which will drive other GPIB adapters to the limits, because binary data of unknown length needs to be transported successfully.

# Setting Parameters

The firmware version from 13th January 2024 onwards has the ability for human readable text based configuration of several parameters.
The previous methods are still supported, but won't be further documented. You can view them in the prior history of this file.

The command parser is quite simple. For that reason follow the exact syntax as shown below. 
Don't add extra spaces or make other modifications or concatenate commands.

## Read termination method

While most GPIB interfaces use the hardware signal `EOI` to signal the end of a message, not all old equipment supports it. Some older instruments even don't have the `EOI` pin implemented and use `\r` or `\n` termination instead.

The following commands are available:

### Set read termination to CR (`\r`):
```
dev.control_in(0xa1, 0x40, 0, 0, 1); # USBTMC pulse indicator request (enables internal command processing)
dev.write('!term cr')
```
Above setting is volatile. To make this a permanent setting call the below mentioned `!term store` command.

### Set read termination to LF (`\n`):
```
dev.control_in(0xa1, 0x40, 0, 0, 1); # USBTMC pulse indicator request (enables internal command processing)
dev.write('!term lf')
```
Above setting is volatile. To make this a permanent setting call the below mentioned `!term store` command.

### Set read termination to EOI only (default setting):
```
dev.control_in(0xa1, 0x40, 0, 0, 1); # USBTMC pulse indicator request (enables internal command processing)
dev.write('!term eoi')
```
Above setting is volatile. To make this a permanent setting call the below mentioned `!term store` command.

### Save read termination setting in EEPROM (making it non-volatile)
```
dev.control_in(0xa1, 0x40, 0, 0, 1); # USBTMC pulse indicator request (enables internal command processing)
dev.write('!term store')
```

### Query current termination setting from device
```
dev.control_in(0xa1, 0x40, 0, 0, 1); # USBTMC pulse indicator request (enables internal command processing)
print(dev.query('!term?'))
```
This returns a text string containing `lf`, `cr` or `eoi`.


## AutoID setting

By default the GPIB adapter tries during power on of the instrument to query the instrument name automatically using `*IDN?` or `ID?` commands.
This is used to build the USB serial number, which finally becomes part of the VISA resource string.

Not all instruments support this `*IDN` / `ID?` query. For this reason this feature can be turned off.
The serial number will then be built based on the GPIB address of the instrument.

### Turn AutoID feature off
```
dev.control_in(0xa1, 0x40, 0, 0, 1); # USBTMC pulse indicator request (enables internal command processing)
dev.write('!autoid off')
```
This setting is stored in EEPROM = non volatile memory, so will survive a power cycle

### Turn AutoID feature on
```
dev.control_in(0xa1, 0x40, 0, 0, 1); # USBTMC pulse indicator request (enables internal command processing)
dev.write('!autoid on')
```
This setting is stored in EEPROM = non volatile memory, so will survive a power cycle

### Turn auto ID on with delay

Some instruments need after turn on some seconds before GPIB is responsive.

With below 3 settings you can set a delay which is applied before the instrument ID is queried after power on.
Note that it will take then also more time, before the USB device is recognized by the PC.

Also this setting is non-volatile.

#### Delay 5 seconds:
```
dev.control_in(0xa1, 0x40, 0, 0, 1); # USBTMC pulse indicator request (enables internal command processing)
dev.write('!autoid slow')
```

#### Delay 15 seconds:
```
dev.control_in(0xa1, 0x40, 0, 0, 1); # USBTMC pulse indicator request (enables internal command processing)
dev.write('!autoid slower')
```

#### Delay 30 seconds:
```
dev.control_in(0xa1, 0x40, 0, 0, 1); # USBTMC pulse indicator request (enables internal command processing)
dev.write('!autoid slowest')
```

### Read the AutoID setting
```
dev.control_in(0xa1, 0x40, 0, 0, 1); # 
print(dev.query('!autoid?'))
```
Returns as text string either: `off`, `on`, `slow`, `slower` or `slowest`.

## Firmware version

Finally I implemented a command to query the USB adapters' firmware version :-)

```
dev.control_in(0xa1, 0x40, 0, 0, 1); # USBTMC pulse indicator request (enables internal command processing)
print(dev.query('!ver?'))
```

## Shorten resource strings (Matlab)

A user discovered that Matlab has a limitation in the VISA resource string length and shared a pull request to reduce the length.
I expose this now for the first time in the baseline firmware with the following options.

This setting is stored in EEPROM = non volatile.

### Limit the USB serial number to a length of 20 characters
```
dev.control_in(0xa1, 0x40, 0, 0, 1); # USBTMC pulse indicator request (enables internal command processing)
dev.write('!string short')
```

### Disable limitation of USB serial number length (default behavior)
```
dev.control_in(0xa1, 0x40, 0, 0, 1); # USBTMC pulse indicator request (enables internal command processing)
dev.write('!string normal')
```

### Query the string length setting.
```
dev.control_in(0xa1, 0x40, 0, 0, 1); # USBTMC pulse indicator request (enables internal command processing)
print(dev.query('!string?'))
```

This returns as text string either "normal" or "short".

## Reset the adapter
```
dev.control_in(0xa1, 0x40, 0, 0, 1); 
dev.write('!reset')
```

Do a reset of the adapter. Note that due to the reset you have to close the VISA session and start a new one.

# If you got here and you want to support me, here is your chance :-)

[![ko-fi](https://ko-fi.com/img/githubbutton_sm.svg)](https://ko-fi.com/J3J7WPWSQ)
