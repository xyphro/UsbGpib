
This new file will in future contain all updates. The latest updates are always on top.

# Recent Updates

## 04th February 2026

The PDF manuals are now also available in Markdown, generated automatically from the same source as the PDF document. This makes the documentation easier to browse, diff, and maintain in version control, while ensuring both formats stay perfectly in sync. -> See [Tutorials](Tutorials/README.md) section.

Furthermore I am releasing Firmware version V2.3. 
Changes:
- It turned out that Keysight Visa in certain versions can crash when a USBTMC device has a serialnumber with double underscore characters. Note, that the USBTMC standard does not disallow for double underscores. The AutoID function can return double underscores for some instruments - illegal characters returned by *IDN? query get replaced by underscores, so with 2 illegal characters you get double underscores as serial number = visa ressource name. I've now included a filter now to suppress double underscores when autoid is enabled.
- I've fixed Issues #96 and #97 reported by user thewon86. Those were USBTMC protocol issues which were never uncovered or effects seen so far, however the implementation was indeed incorrect here.

I updated also slightly the V3 announcement in the main README.MD. Basically: Firmware is ready for betatesting, Hardware arriving soon and gets distributed immediately once it arrives (@Betatesters: You'll get mail before shipping)!

My US distribution partner <a href="https://binho.io/" target="_blank">binho</a> also added UsbGPIB directly to their webshop (<a href="https://binho.io/test-measurement/products/usbgpib-v2/" target="_blank">https://binho.io/test-measurement/products/usbgpib-v2/</a>). This is ideal for US-market: Devices ship directly from San Francisco and are available for a good price! Checkout also their offerings on Host adapters and PCBite!

## 30th December 2025

Updated the "Buy it or build it section" and added a Digikey US link for buying the adapter.

## 30th November 2025

**Just released a new firmware after testing - version 2.2.**

Changes:
* Read termination related:
  * When the "USBTMC native" method to set the termination was once enabled, disabling it did not have effect anymore - it stayed enabled. Only power cycling got it out of that enabled state. Fixed and tested now.
  * Only \n \r and EOI only were supported read terminations. However, as reported in <a href="https://github.com/xyphro/UsbGpib/issues/85" target="_blank">issue 85</a>, there is quite a number of measurement equipment which requires e.g. ETX (0x03) read termination.  
  I removed now this artificial limitation for read termination, such that any read termination character can be set. From 0x00 to 0xff all codes are supported now.  
  For the eeprom non-volatile based method only \r \n and eoi only options are available and there is no plan to extend it to any termination character as this eeprom based method will also get retired for the upcoming V3 UsbGpib adapter.
* GotoLocal synchronization. USBTMC uses 2 different transfer methods for transfering certain types of messages. Control transfers and Bulk transfers. When e.g. sending a USBTMC command like Reading data which uses Bulk transfers followed by another one like GotoLocal which uses control transfers it is important that they also get executed in that order.   
This is something I took deep care off already since long time for other other control transfers, however as it turned out never tested for GoToLocal.   
And things that are not tested don't work, right?   
Michael discovered this during porting of his <a href="https://github.com/VK2BEA/HP8753-Companion" target="_blank">HP8753-Companion</a> to support the UsbGpib V2 adapter which led to me releasing V2.1 to him a while ago.

The issue is fixed now and confirmed working. This V2.2 includes the 2 additional improvements now.

Next release is planned to cover some good improvements I got suggested on AutoID handling.

## 23rd November 2025

I just finalized a user manual for the UsbGPIB V2 adapter, checkout [Tutorials](Tutorials/README.md) section.

In case you miss certain aspects and want me to add them, let me know (Discussion section, Issue section or email to xyphro@gmail.com)


## From Bench to Ready: Buy It, Plug It, Use It!

Tired of tinkering? Want to skip the hassle and get straight to using your GPIB setup?
* I’ve teamed up with Elecrow to bring you ready-to-roll GPIB-USB converters – perfect for those who want results, not a weekend project.
* These adapters come fully assembled, programmed, housed, tested, and ready to go. No fuss. No build. Just plug it in and start working.
* The Adapters match 100% the V2 version described in this repository. 

**Grab yours now:** <a href="https://www.elecrow.com/xyphrolabs-gpibusb.html" target="_blank">https://www.elecrow.com/xyphrolabs-gpibusb.html</a>

Let the data flow begin!

This doesn’t mean I’m going fully commercial - far from it.
Offering pre-built adapters is simply a convenience for those who’d rather skip the build and get straight to using their gear.

All sharing, support, and collaboration will remain fully open, just as before.
This is about choice, not change!

## Coming soon: Native Linux-Gpib support
<small>(Or actually: It's already there!)</small>

A contact with the Linux-Gpib maintainer resulted in a great cooperation with good outcome. We managed to jointly get native linux-gpib support up and alive. Thanks a lot for that, Dave!

We’ve successfully brought full Linux-GPIB compatibility to life. To enable it, you’ll need to flash a dedicated firmware image:  
- File "LinuxGpib.bin" in sub-folder [LinuxGpib firmware](SW/binaries/) will enable it from firmware perspective. Note: This will make the adapter stop from enumerating as normal usbtmc device (you can reflash the normal firmware again later if you like). The upcoming adapter announced a few days ago will have a combined firmware, but the flash of AtMega32U4 was simply "too full".

Stay tuned — I’ll be publishing a detailed guide soon, covering setup instructions and the powerful benefits of going native on Linux.

Just to avoid misunderstandings: The Adapter always worked under Linux as normal UsbTmc device as it does on any other operating system, but was not supported by the Linux-GPIB library so far. So software packages using linux-GPIB can be used without modification in future.

Would btw. love to get similar integration also in Visa Libraries - let's see what time will bring :-)

## Update on Beta Tester program on new upcoming "V3" Adapter with Ethernet+PoE + High speed USB

I managed to find a decent amount of betatesters for the new upcoming HW. Much more as the originally anticipated 3 persons. Thanks a lot for all who volunteered! 

<img src="https://raw.githubusercontent.com/xyphro/UsbGpib/master/pictures/NewHighSpeedEthernetUsbVersion.png" width="50%"/>

<small>Status: From the HW side the Ethernet and PoE section is tested successfully. USB was already enabled since long time.
SW side: The USB functionality is fully implemented and first stress testing shows positive results with no lockups. The Ethernet stack is up and running too, but further implementation and testing work is still to be done.</small>

# 21st April '25 update

Today it's time for another release - version V2.0. 

While writing this and looking at this readme.md file, I certainly think that I should clean it up in future - it is not well structured for new users. Sorry for this, but I have this on my radar and will improve. Whenever I have more time I will also extend it with usage examples / small "trainings", especially for those who are completely new to GPIB and controlling instruments.

I had btw. some occasions where users had issues flashing the device. Many of them were caused by the fact that the .hex and .bin files were saved over the GitHub web interface by rightclicking and selecting "Save as". This will result in a HTML page being saved which will not flash well :-)
The safest option is to do a git clone or to download this whole github repository as .zip file.

Here the release V2.0 details:
- **Bugfix:** the options autoid slower and autoid slowest did not work. This is addressed and I test those settings now also properly in release testing, promised :-) The issue was caused in my super slim parser implementation for custom internal commands and "autoid slow" and "autoid slower" start with the same letters causing hickups.
The Windows GUI is also updated to expose those 2 autoid settings - they were not included before.
As general good recommendation, I still recommend to turn autoid feature off, as it limits the measurement instruments which are used - Instruments HAVE to support *IDN? query for it to work well, which is for many old equipment not the case and for such old equipment errors will be flagged after the instrument is turned on.
- **New feature:** I enabled a new method to set the READ termination. One that is fully compliant with usbtmc standard and does not need a workaround with pulse indicator requests. I don't know why I did not spot this earlier, but a discussion with the usbtmc linux kernel mode driver maintainer exposed this to me. When using direct access to usbtmc kernel mode driver you can use the <a href="https://github.com/dpenkler/linux-usbtmc?tab=readme-ov-file#ioctl-to-control-setting-eom-bit" target="_blank">USBTMC_IOCTL_EOM_ENABLE</a> IOCTL to set the read termination. When using a VISA layer, you can set the read termination by setting the VI_ATTR_TERMCHAR_EN and VI_ATTR_TERMCHAR attributes (see below for more details). 
The previous method of setting read termination is not modified in any way, this just gives an additional method to set the readtermination.
- **Bugfix / Improvement:** The pulse indicator request function did use a blocking delay to pulse the LED. This delayed USB side handling. In some cases this could trigger timeouts. The LED blinking is now handled fully asynchronous and those timeouts will not occur anymore.

# 6th April '25 update

Time to release a bugfix (reported by rapgenic #80): V1.9 of the firmware fixes an issue where the "autoid slow" setting was not applied properly.

# 12th January '25 update

## New Gui available! (Windows)

I implemented a small GUI to change the non-volatile settings of the GpibUSB adapter. Sorry - it's windows only :-)
It is a single .exe file without external dependencies. You can just download it, run it and immediately change settings.

<img src="https://raw.githubusercontent.com/xyphro/UsbGpib/master/pictures/UsbGpibGUI.png" width="80%"/>

Please download it from this folder: [SW/UsbGpibGUI](SW/UsbGpibGUI)

The source code is included in the src sub-directory.

## Python GUI (Linux)

UsbGpib user Jim Houston created an impressive pure Python GUI similar to mine - and I believe he began working on it even before I released mine. His GUI stands out for its versatility and platform independence, requiring only Python to run. It can be executed directly with Python 3 and should run on most platforms.

For details about it, have a look into this discussion thread: [https://github.com/xyphro/UsbGpib/discussions/55](https://github.com/xyphro/UsbGpib/discussions/55)

The GUI can be found here including documentation: [SW/NiceGUI](SW/NiceGUI)

## New Firmware

I release also herewith Version 1.8 of the firmware. 
Mainly a bug-fix was done, that the string setting now also returns short, when it is actually set to short. (issue report #59).

# 13th January '24 update

New year, new update :-)

I realized a user suggestion, which is a better human readable interface to the internal instrument settings.
This also exposes new functionality, e.g. shortening of the visa resource string, adjustable delayed application of AutoID featured, ...
The read termination setting can now be set in a volatile way and also be read back.

A slight issue sneaked in also and got fixed: After an instrument clear the next GPIB transfer timed out. Nobody found it so far though :-) But I fixed it.

Have fun using this!
(would anybody volunteers to make a simple cross platform gui for the non volatile settings :-) ?)

# 19th December '23 update

Finally I made it - the new HW design is on this page. As I had to restructure the repository a bit it took quite a while.
The software image is always the same and runs on both HW revisions.

<img src="https://raw.githubusercontent.com/xyphro/UsbGpib/master/pictures/Upcoming_Rev2.png" width="40%"/>

I also have put a binary for a new Firmware image in there. I am still busy optimizing certain things but want to give you a try before doing a next release.
Please if you test it: Feed it back to me - does it work / does it break anything -> sharing is caring. 

Mail me at xyphro@gmail.com or raise an issue under the issue section.

[FW image location: SW/binaries](SW/binaries)

- 488.2 support further rolled out. This means Status bytes are automatically read out and reported via interrupt transfers to PC as foreseen in the standard.
- FASTER (!) 7 x write speed improvement and 4.x times read speed improvement. As reference: On an FSW I get 310kBytes/s as write transfer speed and 240kBytes/s as read transfer speed.
- a fix for read status byte readout leading to issues on my CMU200
- several smaller race conditions identified and fixed. A lot of work went into stress testing 488.2.

As said, I feel this is the best Firmware image so far, but I changed so much that I want to go first for this small beta test phase asking you explicitly for positive and negative feedback.

---

This page is from XyphroLabs UsbGpib project: <a href="https://github.com/xyphro/UsbGpib" target="_blank">https://github.com/xyphro/UsbGpib</a>