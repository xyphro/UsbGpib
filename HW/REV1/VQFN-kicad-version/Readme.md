# About

These files are a reimplementation of the original PCB using the VQFN-44 packaged ATMEGA32U4 part.  This was ``inspired'' by the fact that during the 2021 chip shortage the TQFP part was hard to come by.

# Notes and Differences

- The board layout is new, but the outline was made to match the original, as were the USB and GPIB port placements, so it should be compatible with the case still.
- The layout was done in KiCAD.  The included files are in **KiCAD version 6**.  As of this writing 6.0 has just been released so most users don't have it.  However it will play nice with KiCAD 5 if you already have that installed, ie you can have them both installed without extra work.  KiCAD 6 was used because the new file format keeps symbols embedded making it easier to share.
- The component part numbers may be different.  This was done to enable fabrication by JLCPCB.  Functionally they should be the same.  It should be compatible with the same USB and GPIB part numbers.
- The component designators should all be the same, at least for the passive components.
- The LED is now on the back side to eliminate the need for any front side components (excluding the through hole connectors).  A second LED footprint was added in parallel with the first and labeled D2.  It can be optionally populated (I recommend removing D1) if top side LED placement is desired.  On the schematic and BOM I also raised the resistor value because the original one same low, but feel free to pick whatever you like during assembly.
- The GPIB part has incorrect pin numberings but the connections are right.  Don't get distressed if the schematic has funny pin numberings, You can blame mouser or whoever they partner with for footprints.

