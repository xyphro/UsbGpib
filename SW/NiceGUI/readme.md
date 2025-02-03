# UsbGpib Python configuration scripts.

These scripts were contributed by [Jim Houston](https://github.com/jimthouston).

The usbgpib_conf.py script is a simple command line interface to
set the UsbGpib parameters.  The usbgpib_gui.py is a [NiceGUI](https://nicegui.io/) based
GUI to do the same thing.
These have been tested with Ubuntu 24.04.  

The other dependency is the [Python USBTMC library](https://github.com/python-ivi/python-usbtmc).  
This library was choosen over pyvisa because it has the pulse command (control_in) which is not implemented in the pyvisa-py backend.
I (jim) believe that both of these packages have support for Windows and MacOS but I have only tested on Linux.

NiceGUI uses the web browser as the frontend for the GUI.  Running the
usbgpib_gui.py script will launch the browser and run the GUI.
Here is a screenshot running in this mode.

<img src="https://raw.githubusercontent.com/xyphro/UsbGpib/master/pictures/nicegui2.png" width="80%"/>

# Setup

To use Python-USBTMC on Linux you need to create a udev rule to create
the device files e.g. `/dev/usbtmc1`.

Add the file `/etc/udev/rules.d/9-usbtmc.rules`:
```
SUBSYSTEM=="usb", ATTR{idVendor}=="03eb", ATTR{idProduct}=="2065", MODE="0660", GROUP="plugdev"
KERNEL=="usbtmc*", MODE="0660", GROUP="plugdev"
```

This allows members of the plugdev group to access the UsbGpib.

Here is the minimum setup needed to run the GUI installing into
a Python virtual environment:

```
python3 -m venv venv
source venv/bin/activate
pip3 install nicegui
pip3 install python-usbtmc pyusb
```

This installs the python packages in the sub-directory venv.  The
activate selects this environment.  To run the GUI use the command 
python3 usbgpib_gui.py`.

# Setup for Native mode.

NiceGUI also has the option to use pywebview to display the GUI in a native window.  To use this mode requires a more complicated set of dependencies.  Here is the setup which worked on Ubuntu 24.04.

```
python3 -m venv venv
source venv/bin/activate
pip3 install python-usbtmc pyusb
pip3 install nicegui pywebview qtpy pyqt5 PyQtWebEngine
sudo apt install python3-gi python3-gi-cairo gir1.2-gtk-3.0 gir1.2-webkit2-4.1
sudo apt-get install ^libxcb.*-dev libx11-xcb-dev libglu1-mesa-dev libxrender-dev libxi-dev libxkbcommon-dev libxkbcommon-x11-dev
```
To use native mode you need to edit the script.  The last two lines of the script are calls to ui.run().  Uncomment the either line to select the mode.
Running the script with native mode selected gives a window like this:

<img src="https://raw.githubusercontent.com/xyphro/UsbGpib/master/pictures/nicegui1.png" width="80%"/>

# usbgpib_conf.py

This command line script only requires Python-USBTMC.  
Here is sample output displaying the configuration and setting a termination
parameter:
```
(venv) jhouston@T5610:~/UsbGpib_new$ python3 usbgpib_conf.py
UsbGpib config for GPIB_22_342313232313
!autoid? off
!term? cr
!string? normal
!ver? V1.6
UsbGpib config for HEWLETT-PACKARD_6642A_0_fA.01.08sA.01.03pA.01.06
!autoid? on
!term? eoi
!string? normal
!ver? V1.8
(venv) jhouston@T5610:~/UsbGpib_new$ python3 usbgpib_conf.py -s GPIB_22_342313232313 term eol
!term eol
```
