# xyphro UsbGpib HW REV 1

This is the original HW design.

It comes in 2 flavors:
- An eagle design which is the root of all designs and my own (xyphro) design.
- A user contributed Kicad design which is also confirmed working correctly.

The [software](../../SW) will always stay the same, no matter if REV1 or REV 2.

## Assembly process

All files related to HW are located in this folder. Gerber file, BOM, Kicad design files.

## preparing the PCB

Mount all components on the PCB. For REV 1 you can assemble the whole design before putting it together.
I mention this explicitely, because for REV 2 it makes sense to mount the GPIB connector only at a later stage of assembly process.

### Flashing the Microcontroller first time

For initial programming an AVR ISP adapter is needed to program the "Bootloader.hex" file.

It is very important, that the Fuses of the AVR are programmed.

Here an example how to program the bootloader using avrdude (using usbasp programmer):
avrdude -c usbasp -p m32u4 -e -Ulock:w:0x3F:m -Uefuse:w:0xcb:m -Uhfuse:w:0xd8:m -Ulfuse:w:0xff:m
avrdude -c usbasp -p m32u4 -U flash:w:BootLoader.hex

After programming the file, disconnect and connect the device and a USB drive will show up. Copy the TestAndMeasurement.bin file to this USB drive - ideally using the command line. Example: `copy TestAndMeasurement.bin F:\FLASH.BIN`.
On Linux, there is a bug with the LUFA mass storage that means it is required to use `dd if=TestAndMeasurement.bin of=/mnt/FLASH.BIN bs=512 conv=notrunc oflag=direct,sync`.

When done, disconnect and connect USB again and you're ready to use it!

## Building the whole thing together
Preparation:
- Mount the PCB EXCLUDING the GPIB connector.
- Print the housings located in [this folder](../../Housing/Rev1). I print them using 1mm bottom/top/wall thickness, 0.16mm layer height and 30% infil. On my older Ender 5 printer a print takes about 2.5 hours.

Assembly is easy: Insert the fully mounted PCB into the power part of the housing.

Then snap on the top part of the housing. Done :-)


# Updating the firmware at later stages

To enter the boot loader at later stages for updates, short circuit the 2 pins of the ISP header, as shown in below picture for about 3 seconds:

<img src="https://raw.githubusercontent.com/xyphro/UsbGpib/master/pictures/BootLoaderEnterTrick.jpg" width="40%"/>

Afterwards, a USB drive will show up and you can copy the firmware again to the device, as described in the previous section.

