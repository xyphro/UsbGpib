# UsbGpib
Versatile, cheap and portable USB to GPIB converter (USBTMC class based).

If you have a lot of test equipment at home, you might know the issues: Lot's of devices only have GPIB as interface and the generic GPIB adapters and GPIB cables on the market are very expensive and some of them even have many issues, when run under Windows 10 (device driver does not work). 

The adapters are also typically very long, such that they extend the overall length of your test equipment my at least 10cm (~4 inch).

Apart of the 2 very big manufacturer, other GPIB adapters, e.g. with Ethernet or also USB interface are not recognized my normal VISA providers or PyVisa, making the measurement control implementation specific for your GPIB adapter.

I've got frustrated and tried to turn it into something positive.

Some goals of the project were:
- Work based on the standard USBTMC protocol. This allows the GPIB test equipment to look like a USB based measurement device and work flawless with e.g. NI VISA, Labview, Matlab or PyVisa.
- Have a small length - otherwise my eqipment has the risk of falling from the shelf :-) Also the USB cable should connect 90 degree angled, to make it very short.
- It should be cheap but still versatile
- It should support ALL my test equipments, from many different GPIB implementation generations and different GPIB flavors
- The Firmware should be upgradeable over USB
- It should be rock-solid (!) I don't want to end up in a very long measurement beeing interrupted because of a software issue of my USB GPIB converter.
- It should support additional features like serial poll, remote enabling/disabling

All those goals are met :-)

# Hardware

## Microcontroller choice

Allthough I typically would prefer nowadays a nice ARM Cortex M0/3/4/7 controller, there is an issue with it. Available devices support only max. 3.3V supply voltages, such that there would be a requirement for a level shifter towards the GPIB Bus.
GPIB is based on 5V (not exactly true, but a first iteration).

This limited the Microcontroller choice to e.g. AVR or PIC controllers. Because of very good availability I ended up in ATMEGA32U4 controllers.
Apart of the device supporting 5V I/O voltages, it also does not require a regulator to be part of the application - it has an internal 3.3V regulator. This minimizes the full application schematic and BOM.

Apart from that, there is an excellent USB stack available [http://www.fourwalledcubicle.com/LUFA.php](http://www.fourwalledcubicle.com/LUFA.php).

The GPIB side of the schematic can be directly connected to the ATMega32U4 IO pins. The IO pins from the microcontroller side are only set to 2 different states: Tristate (input) or output LOW, to talk over GPIB.








