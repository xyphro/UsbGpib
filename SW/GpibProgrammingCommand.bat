REM This batch file programs the firmware for the Xyphro USB-GPIB adapter in Win10.
REM This batch file requires a copy of avrdude to be installed.
REM This batch file assumes that the .hex files are installed in a subdirectory (any name)
REM avrdude is installed in. 
REM To use this batch file:
REM 1. Install avrdude
REM 2. save the two files TestAndMeasurement.hex  and BootLoader.hex in a sub-directory
REM 3. Run the Win10 command line interface (press Windows key and "R"), enter "cmd" and press return.
REM 4. Change directory to the one that has the the hex files and this batch file.  
REM    This needs to be a sub-directory of the directory with the executable avrdude file.
REM 5. Connect the USB-GPIB adapter to a programmer and to a usb port on the PC running avrdude.
REM 5. Run this script to execute the following command.

..\avrdude -c usbasp -p m32u4 -e -Ulock:w:0x3F:m -Uefuse:w:0xcb:m -Uhfuse:w:0xd8:m -Ulfuse:w:0xff:m -U flash:w:TestAndMeasurement.bin -U flash:w:BootLoader.hex
echo
pause
REM This command will install the firmware using a usbasp programmer.  
REM Using a different programmer will require a different parameter.