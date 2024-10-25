/*
             LUFA Library
     Copyright (C) Dean Camera, 2019.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org

     Modified for USB-GPIB adapter
*/

/*
  Copyright 2019  Dean Camera (dean [at] fourwalledcubicle [dot] com)

  Permission to use, copy, modify, distribute, and sell this
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in
  all copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaims all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

/*  	LUFA library modified for USB-GPIB adapter
	software developed by Xyphro 2022-2024
	https://github.com/xyphro/UsbGpib

	Hardware ###########
	Version 1 : adapter with single red LED
	Version 2 : adapter with one red and one green LED

	Xyphro Version 2 hardware design includes 1x grounded pin to  provide hardware signal 
	of 2x LEDs to software.
	This feature was not used in this software because it would require a test on code to test for 
	ver 1 or ver 2 LED.  This would take up machine cycles in places where that is not desirable.

	Software ###########
	Modified by D F Conway 2024 to be compatible with v1 or v2 hardware
	Compiler directives are used to output executable code for v1 or v2.
	This method used to allow a single version of source code to support 
	v1 and v2 hardware.

	Updates to the single source code will apply to both versions.
	Both version 1 & 2 will run on version 1 & 2 hardware.
	Only the LED indications may not act as expected.
	Software Version 2.0 23 Oct 2024 by D F Conway


	LED Signals  ###################

	LED green ON : GPIB device connected
	LED green fast flash : GPIB device actively communicating
	LED red slow flash : USB connected but not enabled

*/

/*
	TIP #######
	HP 3457A: END,ALWAYS => turns on EOI signal!
*/

#include "TestAndMeasurement.h"
#include "gpib.h"
#include <avr/eeprom.h>
#include "global.h"
#include "miniparser.h"

/*  Defines code for version 2 hardware.
    Comment out the #define VER2 line to compile software for
    single LED, version 1 hardware
*/
#define  VER2

  /* Turn LEDs ON/OFF by switching port Hi/Lo */
  #define LED_RED_ON  { PORTF |=  (1<<5); }
  #define LED_RED_OFF { PORTF &= ~(1<<5); }
  #define LED_RED_TGL { PORTF ^=  (1<<5); }   // Toggle RED LED 


  #define LED_GRN_ON  { PORTF |=  (1<<4); }
  #define LED_GRN_OFF { PORTF &= ~(1<<4); }

  /* Set LED pins as always HI outputs to enable.   */
  #define LED_RED_INIT  { (DDRF |= (1<<5)); }
  #define LED_GRN_INIT  { (DDRF |= (1<<4)); }


static inline void TMC_Task(void);

/*  Contains the (usually static) capabilities of the TMC device. This table is requested by the
 *  host upon enumeration to give it information on what features of the Test and Measurement USB
 *  Class the device supports.
 */
TMC_Capabilities_t Capabilities =
	{
		.Status     = TMC_STATUS_SUCCESS,
		.TMCVersion = VERSION_BCD(1,0,0),

		.Interface  =
			{
				.ListenOnly             = false,
				.TalkOnly               = false,
				.PulseIndicateSupported = true,
				.Reserved				= 0,
			},

		.Device     =
			{
				.SupportsAbortINOnMatch = false, // false
				.Reserved = 0,
			},
		.Reserved2 = {0, 0, 0, 0, 0, 0},
		.bcdUSB488 = VERSION_BCD(1,0,0),
		.USB488IfCap1 =
			{
				.SupportTrigger         = 1, //0
				.SupportRenControl      = 1,
				.Is488IF                = 1, // was:0
				.Reserved				= 0,
			},
		.USB488IfCap2 = // device capabilities
			{
				.DT1Capable          	= 1,//0 => Device trigger no capability / full capability
				.RL1Capable		= 1,//0 => Remote Local   no capability / full capability
				.SR1Capable		= 1,//0 => service request 
				.MandatorySCPI		= 0,//0
				.Reserved		= 0,//0
			},
		.Reserved3 = {0, 0, 0, 0, 0, 0, 0, 0},
	};

/** Current TMC control request that is being processed */
static uint8_t RequestInProgress = 0;

/** Stream callback abort flag for bulk IN data */
static bool IsTMCBulkINReset = false;

/** Stream callback abort flag for bulk OUT data */
static bool IsTMCBulkOUTReset = false;

/** Flag that a selective device clear should be executed */
static bool handleSDC = false;

/** Flag that a local lockout should be executed */
static bool handleLocalLockout = false;

/** Flag that a goto local should be executed */
	static bool handleGoToLocal = false;

/** Flag that a ReadStatusByte should be executed */
static bool    handleReadStatusByte = false;
static uint8_t handleReadStatusByte_btagvalue = 0;

/** Flag that a status byte should be read */
static bool handleRSTB = false;
static uint8_t RSTB_btag;
static uint8_t RSTB_status;

/** a global flag to indicate that a gpib write transfer triggered via usb is active
  * This is required to solve a prevent read status byte triggered by control transfers to
  * result in unsynchronized GPIB transfers.
  */
bool gpib_write_is_busy = false;

/** Last used tag value for data transfers */
static uint8_t CurrentTransferTag = 0;

/** Length of last data transfer, for reporting to the host in case an in-progress transfer is aborted */
static uint16_t LastTransferLength = 0;

/** This will be set true after a indicator pulse command is received. If the next GPIB command starts with '!', a parameter has to be set */
static bool s_nextwrite_mightbeparameterset = false;

static uint32_t s_remaining_bytes_receive=0;

static uint8_t gpib_addr = 1;

/** This readbuffer is used to buffer GPIB read data.
 *  As the message header in USBTMC indicates how much data is read and the instrument
 *  does not tell us over GPIB how much data it is, we have to buffer it in ram
 *  if the data transfer over USB should be efficient.
 * The size is decreased by 12 because this is the TMC header length for the first Bulk packate.
 * It is not necessary to substract it, but it does not make sense performance wise.
 */
static uint8_t readbuffer[1024 - 12];

/**
 * When SRQ is asserted the status byte is automatically read and transfered as interrupt transfer to Host.
 * However, the Host Visa might not have events enabled and then a status byte can be lost.
 * The Visa will for that reason issue normally after that interrupt transfer a control in transfer to read
 * the status byte again. Or this is issued manually by the application.
 * The below byte is ored during control transfer to the status byte and then cleared after read to ensure
 * that no data is lost.
 */
static uint8_t srq_statusbyte = 0x00;


static volatile bool transfer_busy = false;

/*
#define FLASH_SIZE_BYTES         32768
#define BOOTLOADER_SEC_SIZE_BYTES  4096

uint32_t Boot_Key ATTR_NO_INIT;
#define MAGIC_BOOT_KEY            0xDC42ACCA
#define BOOTLOADER_START_ADDRESS  (FLASH_SIZE_BYTES - BOOTLOADER_SEC_SIZE_BYTES)
void Bootloader_Jump_Check(void) ATTR_INIT_SECTION(3);
void Bootloader_Jump_Check(void)
{
    // If the reset source was the bootloader and the key is correct, clear it and jump to the bootloader
    if ((MCUSR & (1 << WDRF)) && (Boot_Key == MAGIC_BOOT_KEY))
    {
        Boot_Key = 0;
        ((void (*)(void))BOOTLOADER_START_ADDRESS)();
    }
}
*/

/* Buffer for responses from internal command queries*/
uint8_t internal_response_buffer_rpos = 0;
uint8_t internal_response_buffer_len = 0;
uint8_t internal_response_buffer[8];

void Jump_To_Bootloader(void)
{
    // If USB is used, detach from the bus and reset it
    USB_Disable();

    // Disable all interrupts
    cli();

    // Wait two seconds for the USB detachment to register on the host
    Delay_MS(2000);

    // Set the bootloader key to the magic value and force a reset
    //Boot_Key = MAGIC_BOOT_KEY;
    wdt_enable(WDTO_250MS);
    Delay_MS(500);
	((void (*)(void))0x7000)();
    for (;;);
}

/** Main program entry point. This routine contains the overall program flow, including initial
 *  setup of all components and the main program loop.
 */

uint32_t cnt=0;

bool tmc_gpib_write_timedout(void)
{
	USB_USBTask();
	return IsTMCBulkOUTReset | IsTMCBulkINReset;
}

bool tmc_gpib_read_timedout(void)
{
	USB_USBTask(); /* it is safe to handle the gpib read timeout during usb read transfers - readstatusbyte cannot cause a race condition here */
	return IsTMCBulkINReset | IsTMCBulkOUTReset;
}



uint32_t timeout_val;

void timeout_start(uint32_t timeout)
{
	timeout_val = timeout;
}

bool is_timedout(void)
{
	_delay_us(10);
	if (timeout_val == 0)
		return true;

	timeout_val--;
	return false;
}

static void TMC_SetInternalSerial(bool addGPIBAddress);

/* returns TRUE only if a GPIB device responses to any GPIB address */
bool findGpibdevice(void)
{
	uint8_t addr;
	bool devicepresent;

	//gpib_interface_clear();

	devicepresent = false;
	addr = gpib_search();
	devicepresent = (addr < 255);
	if (addr >= 255)	/* fallback to GPIB address 1, if no device was found */
		addr = 1;
	gpib_addr = addr; /* set global GPIB address to found address*/

	return devicepresent;
}

/* returns TRUE, if a string was received over GPIB */
bool identifyGpibDevice(void)
{
	uint8_t c, len, hascomma;
	bool    eoi, timedout;
	bool    gotStringViaGPIB;

	gotStringViaGPIB = true;

	hascomma = false; /* does the response contain a , character? */

	tmc_serial_string.Header.Size = 0;

	timeout_start(100000); /* 1s timeout*/
	gpib_make_listener(gpib_addr, is_timedout);
	if (timeout_val != 0) gpib_writedat('*', false, is_timedout);
	if (timeout_val != 0) gpib_writedat('I', false, is_timedout);
	if (timeout_val != 0) gpib_writedat('D', false, is_timedout);
	if (timeout_val != 0) gpib_writedat('N', false, is_timedout);
	if (timeout_val != 0) gpib_writedat('?', false, is_timedout);
	if (timeout_val != 0) gpib_writedat('\n', true, is_timedout);
	gpib_untalk_unlisten(is_timedout);

	if (timeout_val != 0)
	{
		timeout_start(100000); /* 1s timeout*/
		gpib_make_talker(gpib_addr, is_timedout);
		len = 0;
		do
		{
			c = gpib_readdat(&eoi, &timedout, is_timedout);
			hascomma = hascomma || (c == ',');
			if ( (c=='\"') || (c=='*') || (c=='/') || (c=='\\') || (c==':') || (c=='?') || (c==' ') || (c==',') || (c=='&') ) /* YEP, a comma and amphersand is allowed in USBTMC spec, but R&S SW does not like this... */
				c='_';
			if ( (c >=32) && (c <=126))
				tmc_serial_string.UnicodeString[len++] = cpu_to_le16(c);
		}
		while ((len < TMC_MAX_SERIAL_STRING_LENGTH) && (!timedout) && (!eoi));
		/* strip away spaces at end */
		while ((tmc_serial_string.UnicodeString[len-1] == '_') && (len > 1))
			len--;
		tmc_serial_string.Header.Size = len*2 + sizeof(USB_Descriptor_Header_t);

		gpib_untalk_unlisten(is_timedout);

		if ( (timeout_val == 0) || (len==0) ) /* no response to *IDN? string*/
		{ /* so try out ID? query */
			timeout_start(100000); /* 1s timeout*/
			gpib_make_listener(gpib_addr, is_timedout);
			if (timeout_val != 0) gpib_writedat('I', false, is_timedout);
			if (timeout_val != 0) gpib_writedat('D', false, is_timedout);
			if (timeout_val != 0) gpib_writedat('?', false, is_timedout);
			if (timeout_val != 0) gpib_writedat('\n', true, is_timedout);
			gpib_untalk_unlisten(is_timedout);
			if (timeout_val != 0) 
			{
				timeout_start(100000); /* 1s timeout*/
				gpib_make_talker(gpib_addr, is_timedout);
				len = 0;
				do
				{
					c = gpib_readdat(&eoi, &timedout, is_timedout);
					hascomma = hascomma || (c == ',');
					if ( (c=='\"') || (c=='*') || (c=='/') || (c=='\\') || (c==':') || (c=='?') || (c==' ') || (c==',') || (c=='&'))
						c='_';
					if ( (c >=32) && (c <=126) )
						tmc_serial_string.UnicodeString[len++] = cpu_to_le16(c);
				}
				while ((len < TMC_MAX_SERIAL_STRING_LENGTH) && (!timedout) && (!eoi) && (c != '\r') && (c != '\n'));
				/* strip away spaces at end */
				while ((tmc_serial_string.UnicodeString[len-1] == '_') && (len > 1))
					len--;
				tmc_serial_string.Header.Size = len*2 + sizeof(USB_Descriptor_Header_t);
				gpib_untalk_unlisten(is_timedout);
			}
		}


		if (!hascomma)
			if ( (tmc_serial_string.UnicodeString[0] == 'H') &&
				 (tmc_serial_string.UnicodeString[1] == 'P') &&
				 (tmc_serial_string.UnicodeString[2] >= '0') &&
				 (tmc_serial_string.UnicodeString[2] <= '9')     )
			{
				hascomma = true;
			}


		if ((timeout_val == 0) || (len == 0)  || (!hascomma) ) /* timeout happened or length is 0 => build a serial number based on GPIB address */
		{
			TMC_SetInternalSerial(true);
			gotStringViaGPIB = false;
		}
	}
	else
	{ /* no gpib address found => use normal serial number */
		TMC_SetInternalSerial(true);
		gotStringViaGPIB = false;
	}

	//TMC_SetInternalSerial(false);

	gpib_ren(false);
	_delay_ms(100);
	gpib_ren(true);
	return gotStringViaGPIB;
}




static void TMC_SetInternalSerial(bool addGPIBAddress)
{
	uint_reg_t CurrentGlobalInt = GetGlobalInterruptMask();
	uint8_t    len;
	GlobalInterruptDisable();

	uint8_t SigReadAddress = 0x0E;
	tmc_serial_string.Header.Type = DTYPE_String;

	len = 0;
	if (addGPIBAddress)
	{
		tmc_serial_string.UnicodeString[len++] = cpu_to_le16('G');
		tmc_serial_string.UnicodeString[len++] = cpu_to_le16('P');
		tmc_serial_string.UnicodeString[len++] = cpu_to_le16('I');
		tmc_serial_string.UnicodeString[len++] = cpu_to_le16('B');
		tmc_serial_string.UnicodeString[len++] = cpu_to_le16('_');
		tmc_serial_string.UnicodeString[len++] = cpu_to_le16('0' + (gpib_addr/10));
		tmc_serial_string.UnicodeString[len++] = cpu_to_le16('0' + (gpib_addr%10));
		tmc_serial_string.UnicodeString[len++] = cpu_to_le16('_');
	}
	for (uint8_t SerialCharNum = 0; SerialCharNum < (80 / 4); SerialCharNum++)
	{
		uint8_t SerialByte = boot_signature_byte_get(SigReadAddress);

		if (SerialCharNum & 0x01)
		{
			SerialByte >>= 4;
			SigReadAddress++;
		}

		SerialByte &= 0x0F;

		tmc_serial_string.UnicodeString[len++] = cpu_to_le16( (SerialByte >= 10) ?
												                        (('A' - 10) + SerialByte) : ('0' + SerialByte) );
	}
	tmc_serial_string.Header.Size = len*2 + sizeof(USB_Descriptor_Header_t);

	SetGlobalInterruptMask(CurrentGlobalInt);
}

static inline void check_bootloaderEntry(void)
{
	if ( !(PINB & (1<<2)) ) /* check if PB2 is LOW*/
	{
		Jump_To_Bootloader();
	}
}

void eeprom_update_if_changed(uint16_t addr, uint8_t value)
{
	uint8_t oldval;
	eeprom_busy_wait();
	oldval = eeprom_read_byte((uint8_t*)addr); /* read previous gpib address */
	if (oldval != value)
	{
		eeprom_busy_wait();
		eeprom_write_byte((uint8_t*)addr, value);
	}
}

int main(void)
{
	uint8_t prevaddr;

	//mcusr_mirror = MCUSR;
	MCUSR = 0;
	wdt_disable();

	PORTB |=  (1<<2); /* PB2 = PULLUP */
	DDRB  &= ~(1<<2); /* PB2 = input*/

	SetupHardware();

	gpib_init();

	/* apply settings from eeprom */
	eeprom_busy_wait();
	gpib_set_readtermination(eeprom_read_byte((const uint8_t *)105));

	GlobalInterruptEnable();


	/* While no GPIB equipment is connected, medium fast flash red LED at 2Hz, green LED OFF.  version 1 and 2. */
	while (!gpib_is_connected())
	{
		#ifdef VER2
		  LED_GRN_OFF
		#endif
		LED_RED_ON
		_delay_ms(200);
		LED_RED_OFF
		_delay_ms(300);
		check_bootloaderEntry();
	}

	uint8_t autoid = eeprom_read_byte((const uint8_t *)104);
	if ( (autoid >= 0x02) && (autoid <= 0x04) ) // check if SLOW AUTOID mode is activated
	{ // yes, it is active, so wait 10 seconds
		uint8_t seconds;
		seconds = 5;
		if (autoid == 0x03)
			seconds = 15;
		if (autoid == 0x04)
			seconds = 30;
		for (uint8_t i=0; i<seconds; i++)
		{
			Delay_MS(1000);
		}
	}

	/* physically GPIB is connected, now check if any GPIB address is responsive */
#ifndef SPEEDTEST_DUMMY_DEVICE
//asdf
	while (!findGpibdevice())
	{  // if the Gpib device is unplugged, turn off green LED (ver2) or show a short red flash at 1Hz ( ver 1 )
               #ifdef VER2
                  LED_GRN_OFF
                #else
 		  _delay_ms(100);
		  LED_RED_ON;
		  _delay_ms(1000);
		  LED_RED_OFF;
		#endif
		if (!gpib_is_connected()) /* we want to reset here if the device is unplugged */
		{   // turn off the green LED if the instrument is unplugged (ver 1) and set the watch dog timer to 250ms.
                        #ifdef VER2
                          LED_GRN_OFF
			#else
  			  LED_RED_OFF;
                        #endif
			_delay_ms(500);
			wdt_enable(WDTO_250MS);
			while (1);
		}
		check_bootloaderEntry();
	}; /* Identify the GPIB Address of the connected GPIB device */
#endif
	eeprom_busy_wait();
#ifdef SPEEDTEST_DUMMY_DEVICE
	if ( false )
#else
	if (autoid != 0x01) // check if AUTOID feature is enabled
#endif
	{
		/* found a responsive GPIB address, now setup USB descriptor with *IDN? or ID? command response */
		eeprom_busy_wait();
		prevaddr = eeprom_read_byte((uint8_t*)0); /* read previous gpib address */
		if (identifyGpibDevice())
		{ /* received a string over GPIB => Store it in EEPROM, if it changed */
			uint8_t *pdat, i;

			/* update gpib address and usb string descriptor in eeprom */
			eeprom_update_if_changed(0, gpib_addr);
			pdat = (void *)&tmc_serial_string;
			for (i=0; i<sizeof(tmc_serial_string); i++)
			{
				eeprom_update_if_changed(1+i, *pdat++);
			}
		}
		else
		{ /* received NO string over GPIB => Check, if the GPIB addr matches the one in eeprom, then report EEProm string! */
			if (prevaddr == gpib_addr)
			{
				uint8_t *pdat, i;

				/* update gpib address and usb string descriptor in eeprom */
				pdat = (void *)&tmc_serial_string;
				for (i=0; i<sizeof(tmc_serial_string); i++)
				{
					*pdat++ = eeprom_read_byte((uint8_t*)(1+i));
				}

			}
		}
	}
	else /* user disabled fully automated detection mode */
	{
		TMC_SetInternalSerial(true);
		gpib_ren(false);
		_delay_ms(100);
		gpib_ren(true);
	}

	/* if activated: shorten the USB serial number string. This is just for Matlab which does not allow resource strings to be longer than 20 */
	if (eeprom_read_byte((const uint8_t *)106) == 0x01) // check if serial string shortening is on
	{
		uint16_t len;

		len = (tmc_serial_string.Header.Size - sizeof(USB_Descriptor_Header_t)) >> 1;
		if (len > 20)
		{
			len = 20;
			tmc_serial_string.Header.Size = len*2 + sizeof(USB_Descriptor_Header_t);
		}
	}

	/* all fine, now kickoff connect to USB to be able to communicate! */
	/* Ver 1&2: turn OFF the red LED to indicate USB is connected  */
 	LED_RED_OFF
	USB_Attach();

	for (;;)
	{
		TMC_Task(); // this task is 9.42us active when nothing is triggered, the rest takes 3.3us
		check_bootloaderEntry();

		if (!gpib_is_connected()) /* check, if gpib is disconnected */
		{ /* when we get here, reset the MCU and disconnect from USB ! It will reconnect once plugged in to GPIB again */
		  /* turn ON the red LED when USB cable is disconnected and set the watch dog timer = 250ms  ver 1 */
			LED_RED_ON
			/* gpib not connected, so turn off green LED */
	                #ifdef VER2
        	          LED_GRN_OFF
                	#endif

			USB_Detach();
			_delay_ms(500);
			wdt_enable(WDTO_250MS);
			while (1);
		}

		USB_USBTask();
	}
}

/** Configures the board hardware and chip peripherals for the demo's functionality. */
void SetupHardware(void)
{
	/* Disable watchdog if enabled by bootloader/fuses */
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	/* Disable clock division */
	clock_prescale_set(clock_div_1);


	/* Hardware Initialization */
	USB_Init();
	USB_Detach();


	/* update the TMC default serial number*/
	TMC_SetInternalSerial(false);

	/* Switch red LED pin to output and turn on ( ver 1 & 2 ) */
	/* Initiate the LED pins*/
 	LED_RED_INIT
	LED_RED_OFF      /* assume USB not connected even though it is */
	#ifdef VER2
	  LED_GRN_INIT  /* Initiate Green LED pin as output only for ver 2 */
	  LED_GRN_OFF   /* assume no gpib connected */
	#endif
}

/** Event handler for the USB_Connect event. This indicates that the device is enumerating via the status LEDs and
 *  starts the library USB task to begin the enumeration and USB management process.
 */
void EVENT_USB_Device_Connect(void)
{
	//LEDs_SetAllLEDs(LEDMASK_USB_ENUMERATING);
}

/** Event handler for the USB_Disconnect event. This indicates that the device is no longer connected to a host via
 *  the status LEDs and stops the USB management and CDC management tasks.
 */
void EVENT_USB_Device_Disconnect(void)
{
	//LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
}

/** Event handler for the USB_ConfigurationChanged event. This is fired when the host set the current configuration
 *  of the USB device after enumeration - the device endpoints are configured and the CDC management task started.
 */
void EVENT_USB_Device_ConfigurationChanged(void)
{
	bool ConfigSuccess = true;

	/* Setup TMC In, Out and Notification Endpoints */
	ConfigSuccess &= Endpoint_ConfigureEndpoint(TMC_NOTIFICATION_EPADDR, EP_TYPE_INTERRUPT, TMC_NOTIFICATION_EPSIZE, 1);
	ConfigSuccess &= Endpoint_ConfigureEndpoint(TMC_IN_EPADDR,  EP_TYPE_BULK, TMC_IO_EPSIZE, 2 );
	ConfigSuccess &= Endpoint_ConfigureEndpoint(TMC_OUT_EPADDR, EP_TYPE_BULK, TMC_IO_EPSIZE, 2 );
}

void TMC_resetstates(void);

/** Event handler for the USB_ControlRequest event. This is used to catch and process control requests sent to
 *  the device from the USB host before passing along unhandled control requests to the library for processing
 *  internally.
 */

void handle_control_Req_ReadStatusByte(void)
{
	uint8_t btag, statusReg;
	uint8_t PrevEndpoint = Endpoint_GetCurrentEndpoint();

	Endpoint_SelectEndpoint(ENDPOINT_CONTROLEP);
	btag = handleReadStatusByte_btagvalue;

	gpib_ren(1); /* ensure that remote control is enabled */
	timeout_start(150000); /* 1.5s timeout*/
	statusReg =  gpib_readStatusByte(gpib_addr, is_timedout);
	statusReg |= srq_statusbyte; // or previously read autoread status byte (Visa will issue a read status byte after receiving a SRQ interrupt transfer)
	srq_statusbyte = 0x00; // clear previously read autoread status byte

	/* Write the request response byte */
	Endpoint_Write_8(TMC_STATUS_SUCCESS);
	Endpoint_Write_8(btag);
	Endpoint_Write_8(statusReg);

	/* prepare interrupt response */
	RSTB_btag = btag;
	RSTB_status = statusReg;
	handleRSTB = true;

	Endpoint_ClearIN();
	Endpoint_ClearStatusStage();

	handleReadStatusByte = false; /* no matter what - we can cancel the pending transfer handling */
	Endpoint_SelectEndpoint(PrevEndpoint);
}

void EVENT_USB_Device_ControlRequest(void)
{
	uint8_t TMCRequestStatus = TMC_STATUS_SUCCESS;

	if ( ((USB_ControlRequest.wIndex == INTERFACE_ID_TestAndMeasurement) && ((USB_ControlRequest.bmRequestType & REQREC_INTERFACE)!=0)) ||
	     (((USB_ControlRequest.wIndex == TMC_IN_EPADDR) || (USB_ControlRequest.wIndex == TMC_OUT_EPADDR)) && ((USB_ControlRequest.bmRequestType & REQREC_ENDPOINT)!=0))     )
	{
		/* Process TMC specific control requests */
		switch (USB_ControlRequest.bRequest)
		{
			case Req_ReadStatusByte:
				Endpoint_ClearSETUP();
				handleReadStatusByte_btagvalue = USB_ControlRequest.wValue;
				if ( !gpib_write_is_busy )
				{ /* no write transfer active, thus handle the request immediately */
					handle_control_Req_ReadStatusByte();
				}
				else
				{ /* a USB triggered GPIB write transfer is active. Mark the handling of this request for later */
					handleReadStatusByte = true;
				}

				break;
			case Req_InitiateAbortBulkOut:
				if (USB_ControlRequest.bmRequestType == (REQDIR_DEVICETOHOST | REQTYPE_CLASS | REQREC_ENDPOINT))
				{
					/* Check that no split transaction is already in progress and the data transfer tag is valid */
					if (RequestInProgress != 0)
					{
						TMCRequestStatus = TMC_STATUS_SPLIT_IN_PROGRESS;
					}
					else if (USB_ControlRequest.wValue != CurrentTransferTag)
					{
						TMCRequestStatus = TMC_STATUS_TRANSFER_NOT_IN_PROGRESS;
					}
					else
					{
						/* Indicate that all in-progress/pending data OUT requests should be aborted */
						IsTMCBulkOUTReset = true;

						/* Save the split request for later checking when a new request is received */
						RequestInProgress = Req_InitiateAbortBulkOut;
					}
					IsTMCBulkOUTReset = true;

					Endpoint_ClearSETUP();

					/* Write the request response byte */
					Endpoint_Write_8(TMCRequestStatus);
					Endpoint_ClearIN();
					Endpoint_ClearStatusStage();
				}

				break;
			case Req_CheckAbortBulkOutStatus:
				if (USB_ControlRequest.bmRequestType == (REQDIR_DEVICETOHOST | REQTYPE_CLASS | REQREC_ENDPOINT))
				{
					/* Check that an ABORT BULK OUT transaction has been requested and that the request has completed */
					if (RequestInProgress != Req_InitiateAbortBulkOut)
					  TMCRequestStatus = TMC_STATUS_SPLIT_NOT_IN_PROGRESS;
					else if (IsTMCBulkOUTReset)
					  TMCRequestStatus = TMC_STATUS_PENDING;
					else
					  RequestInProgress = 0;

					Endpoint_ClearSETUP();

					/* Write the request response bytes */
					Endpoint_Write_8(TMCRequestStatus);
					Endpoint_Write_16_LE(0);
					Endpoint_Write_32_LE(LastTransferLength);

					Endpoint_ClearIN();
					Endpoint_ClearStatusStage();
				}

				break;
			case Req_InitiateAbortBulkIn:
				if (USB_ControlRequest.bmRequestType == (REQDIR_DEVICETOHOST | REQTYPE_CLASS | REQREC_ENDPOINT))
				{
					/* Check that no split transaction is already in progress and the data transfer tag is valid */
					if (RequestInProgress != 0)
					{
						TMCRequestStatus = TMC_STATUS_SPLIT_IN_PROGRESS;
					}
					else if (USB_ControlRequest.wValue != CurrentTransferTag)
					{
						TMCRequestStatus = TMC_STATUS_TRANSFER_NOT_IN_PROGRESS;
					}
					else
					{
						/* Indicate that all in-progress/pending data IN requests should be aborted */
						IsTMCBulkINReset = true;

						/* Save the split request for later checking when a new request is received */
						RequestInProgress = Req_InitiateAbortBulkIn;
					}
					IsTMCBulkINReset = true;


					Endpoint_ClearSETUP();

					/* Write the request response bytes */
					Endpoint_Write_8(TMCRequestStatus);
					Endpoint_Write_8(CurrentTransferTag);

					Endpoint_ClearIN();
					Endpoint_ClearStatusStage();
				}

				break;
			case Req_CheckAbortBulkInStatus:
				if (USB_ControlRequest.bmRequestType == (REQDIR_DEVICETOHOST | REQTYPE_CLASS | REQREC_ENDPOINT))
				{
					/* Check that an ABORT BULK IN transaction has been requested and that the request has completed */
					if (RequestInProgress != Req_InitiateAbortBulkIn)
					  TMCRequestStatus = TMC_STATUS_SPLIT_NOT_IN_PROGRESS;
					else if (IsTMCBulkINReset)
					  TMCRequestStatus = TMC_STATUS_PENDING;
					else
					  RequestInProgress = 0;

					Endpoint_ClearSETUP();

					/* Write the request response bytes */
					Endpoint_Write_8(TMCRequestStatus);
					Endpoint_Write_16_LE(0);
					Endpoint_Write_32_LE(LastTransferLength);

					Endpoint_ClearIN();
					Endpoint_ClearStatusStage();
				}

				break;
			case Req_InitiateClear:

				if (USB_ControlRequest.bmRequestType == (REQDIR_DEVICETOHOST | REQTYPE_CLASS | REQREC_INTERFACE))
				{
					Endpoint_ClearSETUP();
					/* Check that no split transaction is already in progress */
					if (RequestInProgress != 0)
					{
						Endpoint_Write_8(TMC_STATUS_SPLIT_IN_PROGRESS);
					}
					else
					{
						/* Indicate that all in-progress/pending data IN and OUT requests should be aborted */
						IsTMCBulkINReset  = true;
						IsTMCBulkOUTReset = true;
						handleSDC = true; // trigger handling of SDC command to device

						/* Save the split request for later checking when a new request is received */
						RequestInProgress = Req_InitiateClear;
					}


					/* Write the request response byte */
					Endpoint_Write_8(TMCRequestStatus);

					Endpoint_ClearIN();
					Endpoint_ClearStatusStage();
				}

				break;
			case Req_CheckClearStatus:
				if (USB_ControlRequest.bmRequestType == (REQDIR_DEVICETOHOST | REQTYPE_CLASS | REQREC_INTERFACE))
				{
					/* Check that a CLEAR transaction has been requested and that the request has completed */
					if (RequestInProgress != Req_InitiateClear)
						TMCRequestStatus = TMC_STATUS_SPLIT_NOT_IN_PROGRESS;
					else if (IsTMCBulkINReset || IsTMCBulkOUTReset || handleSDC)
						TMCRequestStatus = TMC_STATUS_PENDING;
					else 
					{
						TMCRequestStatus = TMC_STATUS_SUCCESS;
						RequestInProgress = 0;
					}

					Endpoint_ClearSETUP();

					/* Write the request response bytes */
					Endpoint_Write_8(TMCRequestStatus);
					Endpoint_Write_8(0);

					Endpoint_ClearIN();
					Endpoint_ClearStatusStage();
				}

				break;
			case Req_GetCapabilities:
				if (USB_ControlRequest.bmRequestType == (REQDIR_DEVICETOHOST | REQTYPE_CLASS | REQREC_INTERFACE))
				{
					Endpoint_ClearSETUP();

					/* Write the device capabilities to the control endpoint */
					Endpoint_Write_Control_Stream_LE(&Capabilities, sizeof(TMC_Capabilities_t));
					Endpoint_ClearOUT();
				}
				break;
			case Req_IndicatorPulse:
				Endpoint_ClearSETUP();

				/* USBTMC Status response (1 Byte) */
				Endpoint_Write_8(TMC_STATUS_SUCCESS);
				Endpoint_ClearIN();
				Endpoint_ClearStatusStage();

				#ifdef VER2
				/*Turn off green LED = no gpib connected ver 2 */
				   LED_GRN_OFF
				#else
				/*Toggle RED LED, slow flash = no gpib connected  ver 1 */
				   LED_RED_TGL;
				  _delay_ms(250);
				#endif
				s_nextwrite_mightbeparameterset = true;
				break;

			case Req_RenControl:
				if ((USB_ControlRequest.wValue & 0xff) == 1)
				{
					gpib_ren(1);
				}
				else
				{
					gpib_ren(0);
				}
				Endpoint_ClearSETUP();

				/* USBTMC Status response (1 Byte) */
				Endpoint_Write_8(TMC_STATUS_SUCCESS);
				Endpoint_ClearIN();
				Endpoint_ClearStatusStage();
				break;

			case Req_LocalLockout:
				handleLocalLockout = true; // trigger handling of local lockout within TMC_TASK

				Endpoint_ClearSETUP();
				/* USBTMC Status response (1 Byte) */
				Endpoint_Write_8(TMC_STATUS_SUCCESS);
				Endpoint_ClearIN();
				Endpoint_ClearStatusStage();
				break;

			case Req_GoToLocal:
				handleGoToLocal = true; // trigger handling of local lockout within TMC_TASK

				Endpoint_ClearSETUP();
				/* USBTMC Status response (1 Byte) */
				Endpoint_Write_8(TMC_STATUS_SUCCESS);
				Endpoint_ClearIN();
				Endpoint_ClearStatusStage();
				break;
		}
	}
}

/*
Unused function
static uint8_t charToval(char c)
{
	uint8_t val;
	val = 0;
	if ( (c >= '0') && (c <= '9') )
		val = c-'0';
	if ( (c >= 'a') && (c <= 'f') )
		val = c-'a'+10;
	if ( (c >= 'A') && (c <= 'F') )
		val = c-'A'+10;
	return val;
}
*/
void set_internal_response(uint8_t *pdat, uint8_t len)
{
	if (len < sizeof(internal_response_buffer))
	{
		memcpy(internal_response_buffer, pdat, len);
		internal_response_buffer_len = len;
		internal_response_buffer_rpos = 0;
	}
}


/*
Process an internal command. This is triggered, if a indicator pulse command was received, followed
by a write of a command starting with an exclamation mark (!).
Syntax:
!XXYY
XX = index (hex) 00 for GPIB automatic dection:
					YY selects the method:
					0x00 or 0xff => Fully automatic (also try to sense ID? or *IDN? string and use it as serial number string)
					0x01         => Only detect GPIB address automatically and use the GPIB address as serial number string
                 01 for Termination method for READs:
					YY selects the method:
					0x00 or 0xff => EOI termination
					0x01         => EOI or '\n' (LF = linefeed)
					0x02         => EOI or '\r' (CR = carriage return)
*/
void ProcessInternalCommand(uint8_t Length)
{
	bool cmd_executed;

	// clear any old response
	internal_response_buffer_len = 0;
	internal_response_buffer_rpos = 0; 

	parser_reset();
	cmd_executed = false;
	while ( (Length--) && (!cmd_executed) )
	{
		uint8_t dat = Endpoint_Read_8();
		cmd_executed = parser_add( dat );
	}
}

void ProcessSentMessage(uint8_t* const Data, uint8_t Length, bool isFirstTransfer, bool isLastTransfer, bool sendEom, gpibtimeout_t ptimeoutfunc)
{
	uint8_t dat;
	bool timedout, isinternalcommand;

	gpib_write_is_busy = true; /* required to handle read status byte synchronization issue */
	timedout = false;

	dat = Endpoint_Read_8();
	/* check, if this is an internal command (=sidechannel for configuration settings )*/
	isinternalcommand = isFirstTransfer && s_nextwrite_mightbeparameterset && (dat == '!');
	if (isinternalcommand)
	{
		ProcessInternalCommand(Length);
	}
	else
	{

		gpib_ren(1); /* ensure that remote control is enabled */
		/* Turn off the LED ver 1  */
		#ifndef VER2
		  LED_RED_OFF;
		#endif
		if (isFirstTransfer)
			timedout = gpib_make_listener(gpib_addr, ptimeoutfunc);

		if (handleReadStatusByte && !timedout)
		{
			gpib_untalk_unlisten(ptimeoutfunc);
			handle_control_Req_ReadStatusByte();
			timedout = gpib_make_listener(gpib_addr, ptimeoutfunc);
		}

		while ( (Length > 0) && !timedout)
		{
			if (handleReadStatusByte)
			{
				gpib_untalk_unlisten(ptimeoutfunc);
				handle_control_Req_ReadStatusByte();
				timedout = gpib_make_listener(gpib_addr, ptimeoutfunc);
			}
			Length--;
			timedout = gpib_writedat_quick(dat, (Length == 0)  && sendEom, ptimeoutfunc, false);
			if (Length > 0)
			{
				dat = Endpoint_Read_8();
			}

		}

		if (isLastTransfer && !timedout) /* in case of timeout the interface is cleared within the writedat function, no need to untalk!*/
			gpib_untalk_unlisten(ptimeoutfunc);
		#ifndef VER2
  		  LED_RED_ON;  /* Turn on the LED ver 1 */
		#endif
	}
	s_nextwrite_mightbeparameterset = false;
	gpib_write_is_busy = !isLastTransfer; /* required to handle read status byte synchronization issue */

	if (handleReadStatusByte && !timedout)
	{
		if (!(isLastTransfer && !timedout))
			gpib_untalk_unlisten(ptimeoutfunc);
		handle_control_Req_ReadStatusByte();
		if (!(isLastTransfer && !timedout))
			timedout = gpib_make_listener(gpib_addr, ptimeoutfunc);
	}
}

uint16_t GetNextMessage(uint8_t* const Data, uint16_t maxlen, bool isFirstMessage, bool *pisLastMessage, gpibtimeout_t ptimeoutfunc)
{
	uint8_t c;
	uint16_t i;
	bool    Eoi, timedout;

	gpib_ren(1); /* ensure that remote control is enabled */
	/* turn off the led ver 1 */
	#ifndef VER2
	  LED_RED_OFF;
	#endif

	timedout = false;
#ifndef SPEEDTEST_DUMMY_DEVICE
	if (isFirstMessage)
		timedout = gpib_make_talker(gpib_addr, ptimeoutfunc);
#endif

	i = 0;
	Eoi = false;

	while (!Eoi && (i < maxlen) && !timedout)
	{

		c = gpib_readdat_quick(&Eoi, &timedout, ptimeoutfunc, false); 
		Data[i++] = c;
	}

#ifndef SPEEDTEST_DUMMY_DEVICE
	if (Eoi && !timedout) /* in case of timeout, no need to unlisten => interface clear done in readdat function! */
		gpib_untalk_unlisten(ptimeoutfunc);
#endif

	if (timedout) /* in case of timedout, simulate an end of message */
		Eoi = true;
	*pisLastMessage = Eoi;

        #ifndef VER2
          LED_RED_ON;  /* Turn on the LED ver 1 */
        #endif

	return i;
}

bool TMC_LastMessageComplete = true;
bool TMC_eom;
bool TMC_InLastMessageComplete = true;

void TMC_resetstates(void)
{
	internal_response_buffer_len = 0;
	handleGoToLocal = false;
	handleSDC = false;
	handleLocalLockout = false;
	handleReadStatusByte = false;
	TMC_LastMessageComplete = true;
	TMC_InLastMessageComplete = true;
	s_remaining_bytes_receive = 0;
	gpib_interface_clear(); 
//	gpib_untalk_unlisten();
}

/** Speed optimized of Lufa Endpoint_Write_Stream_LE function (it was 1.5 times slower than this one) */
static inline uint8_t Endpoint_Write_Stream_LE_quick(const void* const Buffer,
                            uint16_t Length,
                            uint8_t firstchunksize, uint8_t chunksize)
{
	uint8_t* DataStream      = ((uint8_t*)Buffer);
	uint8_t  ErrorCode;
	uint16_t chunklength = firstchunksize;

	if ((ErrorCode = Endpoint_WaitUntilReady()))
	  return ErrorCode;

	while (Length)
	{
		if (!(Endpoint_IsReadWriteAllowed()))
		{
			Endpoint_ClearIN();

			#if !defined(INTERRUPT_CONTROL_ENDPOINT)
			//USB_USBTask();
			#endif

			if ((ErrorCode = Endpoint_WaitUntilReady()))
			  return ErrorCode;
		}
		else
		{
			if (chunklength > Length)
				chunklength = Length;
			Length -= chunklength;
			while (chunklength--)
				Endpoint_Write_8(*DataStream++);
			chunklength = chunksize;
		}
	}
	return ENDPOINT_RWSTREAM_NoError;
}

/** Function to manage TMC data transmission and reception to and from the host. */
static inline void TMC_Task(void)
{
	bool lastmessage;
	/* Device must be connected and configured for the task to run */
	if (USB_DeviceState != DEVICE_STATE_Configured)
	  return;

	TMC_MessageHeader_t MessageHeader;
	uint8_t             MessagePayload[128], curlen;
	uint16_t            curlen16;


	if (s_remaining_bytes_receive == 0)
	{
		/* handle service request (SRQ line goes down) */
		if (gpib_is_srq_active())
		{ /* SRQ is now and we are outside of a GPIB transfer here. So: Handle it by reading status byte and push it over the interrupt channel! */
			uint8_t statusReg;
			uint8_t notdata[2];
			timeout_start(50000); /* 0.5s timeout*/
			statusReg =  gpib_readStatusByte(gpib_addr, is_timedout) | 0x40;
			srq_statusbyte |= (statusReg & ~0x40);
			notdata[0] = 0x80 | 0x01;
			notdata[1] = statusReg;
			Endpoint_SelectEndpoint(TMC_NOTIFICATION_EPADDR);
			while ( Endpoint_Write_Stream_LE(notdata, sizeof(notdata), NULL) ==
					ENDPOINT_RWSTREAM_IncompleteTransfer)
			{
				if (IsTMCBulkINReset)
				  break;
			}
			Endpoint_ClearIN();
		}

		/* handle actions triggered by control transfer in a synchronous manner here */
		if (handleSDC)
		{
			gpib_ren(1); /* ensure that remote control is enabled */
			timeout_start(50000); /* 0.5s timeout*/
			gpib_sdc(gpib_addr, is_timedout);
			handleSDC = false;
		}

		if (handleLocalLockout)
		{
			timeout_start(50000); /* 0.5s timeout*/
			gpib_localLockout(is_timedout);
			handleLocalLockout = false;
		}

		if (handleGoToLocal)
		{
			timeout_start(50000); /* 0.5s timeout*/
			gpib_gotoLocal(gpib_addr, is_timedout);
			handleGoToLocal = false;
		}


		/* Try to read in a TMC message from the interface, process if one is available */
		if (ReadTMCHeader(&MessageHeader))
		{
			/* Indicate busy */
			//LEDs_SetAllLEDs(LEDMASK_USB_BUSY);

			switch (MessageHeader.MessageID)
			{
				case TMC_MESSAGEID_TRIGGER:
					gpib_ren(1); /* ensure that remote control is enabled */
					timeout_start(50000); /* 0.5s timeout*/
					gpib_trigger(gpib_addr, is_timedout);
					Endpoint_ClearOUT();
					break;
				case TMC_MESSAGEID_DEV_DEP_MSG_OUT:
					s_remaining_bytes_receive = MessageHeader.TransferSize;

					curlen = MIN(TMC_IO_EPSIZE-sizeof(TMC_MessageHeader_t), MessageHeader.TransferSize);

					s_remaining_bytes_receive -= curlen;

					TMC_eom = (MessageHeader.MessageIDSpecific.DeviceOUT.LastMessageTransaction != 0);
					lastmessage =  (s_remaining_bytes_receive==0);
					ProcessSentMessage(MessagePayload, curlen, TMC_LastMessageComplete, lastmessage, TMC_eom && lastmessage, tmc_gpib_write_timedout);

					/* Select the Data Out endpoint, this has to be done because the timeout function cal select the control endpoint */
					Endpoint_SelectEndpoint(TMC_OUT_EPADDR);
					Endpoint_ClearOUT();

					TMC_LastMessageComplete = lastmessage;
					break;
				case TMC_MESSAGEID_DEV_DEP_MSG_IN:
					Endpoint_ClearOUT();

					curlen16 = sizeof(readbuffer);// -1;
					if (curlen16 > MessageHeader.TransferSize)
						curlen16 = MessageHeader.TransferSize;

					/* Check if a response from an internal query is in the buffer */
					if (internal_response_buffer_len) 
					{ // internal response present
						// Add response to buffers... This might look too complicated, but handles also partial reads properly (e.g. responses could be read byte by byte)
						curlen16 = internal_response_buffer_len-internal_response_buffer_rpos; // count of unsent bytes from internal response buffer
						if (curlen16 > MessageHeader.TransferSize)
							curlen16 = MessageHeader.TransferSize;

						MessageHeader.TransferSize = curlen16;
						memcpy(readbuffer, &(internal_response_buffer[internal_response_buffer_rpos]), curlen16);

						internal_response_buffer_rpos += curlen16;
						lastmessage = (internal_response_buffer_rpos >= internal_response_buffer_len);
						if (lastmessage)
						{
							internal_response_buffer_len = 0; // Mark internal response as "sent"
							internal_response_buffer_rpos = 0;
						}
					}
					else
					{ // no internal response present, read from device!
						MessageHeader.TransferSize = GetNextMessage(readbuffer, curlen16, TMC_InLastMessageComplete, &lastmessage, tmc_gpib_read_timedout);
					}
					TMC_InLastMessageComplete = lastmessage;

					MessageHeader.MessageIDSpecific.DeviceOUT.LastMessageTransaction = lastmessage;
					if (!IsTMCBulkINReset)
						WriteTMCHeader(&MessageHeader);

					if (!IsTMCBulkINReset)
					{
						Endpoint_Write_Stream_LE_quick(readbuffer, MessageHeader.TransferSize, TMC_IO_EPSIZE-sizeof(TMC_MessageHeader_t), TMC_IO_EPSIZE);
					}

					/* Also in case of a timeout, the host does not expire a Bulk IN IRP, so we still need to commit an empty endpoint to retire the IRP */
					Endpoint_SelectEndpoint(TMC_IN_EPADDR);
					Endpoint_ClearIN();

					/* commit zero length package in case the last package was exactly the size of the endpoint (Lufa does not handle this) */
					if ( ((MessageHeader.TransferSize + sizeof(TMC_MessageHeader_t)) & (TMC_IO_EPSIZE-1)) == 0)
					{ 
						Endpoint_WaitUntilReady(); // wait until an endpoint buffer got free
						Endpoint_ClearIN();
					}

					if (IsTMCBulkINReset)
					{
						TMC_resetstates();
					}

					break;
				default:
					Endpoint_StallTransaction();
					break;
			}
		}
	}
	else
	{ /* receiving further bytes to be sent over GPIB */
			/* Select the Data Out endpoint */
		Endpoint_SelectEndpoint(TMC_OUT_EPADDR);

		/* Abort if no command has been sent from the host */
		if (Endpoint_IsOUTReceived())
		{
			LastTransferLength = 0;

			curlen = TMC_IO_EPSIZE;
			if (s_remaining_bytes_receive < TMC_IO_EPSIZE)
			{
				curlen = s_remaining_bytes_receive;
			}

			s_remaining_bytes_receive -= curlen;


			lastmessage = (s_remaining_bytes_receive==0);
			TMC_LastMessageComplete = lastmessage;
			ProcessSentMessage(MessagePayload, curlen, false, lastmessage, TMC_eom && lastmessage, tmc_gpib_write_timedout);
			Endpoint_ClearOUT();
		}
	}

	if (handleRSTB)
	{
		uint8_t  notdata[2];

		handleRSTB = false;

		notdata[0] = RSTB_btag | 0x80;
		notdata[1] = RSTB_status;
		Endpoint_SelectEndpoint(TMC_NOTIFICATION_EPADDR);
		while ( Endpoint_Write_Stream_LE(notdata, sizeof(notdata), NULL) ==
				ENDPOINT_RWSTREAM_IncompleteTransfer)
		{
			if (IsTMCBulkINReset)
			  break;
		}
		Endpoint_ClearIN();
	}


	if (IsTMCBulkOUTReset || IsTMCBulkINReset)
	{

		Endpoint_SelectEndpoint(TMC_OUT_EPADDR);
		Endpoint_ClearOUT();
		Endpoint_ClearOUT();

		Endpoint_SelectEndpoint(TMC_IN_EPADDR);
		Endpoint_AbortPendingIN();

		/* SDC has to be sent before clearing state, because SDC request is stopped in reset states */
		if (handleSDC)
		{
			gpib_ren(1); /* ensure that remote control is enabled */
			timeout_start(50000); /* 0.5s timeout*/
			gpib_sdc(gpib_addr, is_timedout);
			handleSDC = false;
		}

		TMC_resetstates();

	}

	/* All pending data has been processed - reset the data abort flags */
	IsTMCBulkINReset  = false;
	IsTMCBulkOUTReset = false;

	transfer_busy = (s_remaining_bytes_receive != 0);
}

/** Attempts to read in the TMC message header from the TMC interface.
 *
 *  \param[out] MessageHeader  Pointer to a location where the read header (if any) should be stored
 *
 *  \return Boolean \c true if a header was read, \c false otherwise
 */
bool ReadTMCHeader(TMC_MessageHeader_t* const MessageHeader)
{
	uint8_t *pdat = (uint8_t *)MessageHeader;

	/* Select the Data Out endpoint */
	Endpoint_SelectEndpoint(TMC_OUT_EPADDR);

	/* Abort if no command has been sent from the host */
	if (!(Endpoint_IsOUTReceived()))
	  return false;

	/* on purpose Endpoint_Read_Stream_LE it not used here, because it handles USB control requests which is useless here */
	for (uint8_t i=0; i<sizeof(TMC_MessageHeader_t); i++)
	{
		*pdat++ = Endpoint_Read_8();
	}

	/* Store the new command tag value for later use */
	CurrentTransferTag = MessageHeader->Tag;

	/* Indicate if the command has been aborted or not */
	return !IsTMCBulkOUTReset;
}

/** Speed optimized of Lufa Endpoint_Write_Stream_LE function (it was 1.5 times slower than this one) */
static inline uint8_t Endpoint_Write_Stream_LE_noUsbTask(const void* const Buffer,
                            uint16_t Length)
{
	uint8_t* DataStream      = ((uint8_t*)Buffer);
	uint8_t  ErrorCode;

	if ((ErrorCode = Endpoint_WaitUntilReady()))
	  return ErrorCode;

	while (Length)
	{
		if (!(Endpoint_IsReadWriteAllowed()))
		{
			Endpoint_ClearIN();

			if ((ErrorCode = Endpoint_WaitUntilReady()))
			  return ErrorCode;
		}
		else
		{
			Endpoint_Write_8(*DataStream++);
			Length--;
		}
	}
	return ENDPOINT_RWSTREAM_NoError;
}

bool WriteTMCHeader(TMC_MessageHeader_t* const MessageHeader)
{
	/* Set the message tag of the command header */
	MessageHeader->Tag        =  CurrentTransferTag;
	MessageHeader->InverseTag = ~CurrentTransferTag;

	/* Select the Data In endpoint */
	Endpoint_SelectEndpoint(TMC_IN_EPADDR);

	/* Send the command header to the host */
	//Endpoint_Write_Stream_LE_quick(readbuffer, MessageHeader.TransferSize, TMC_IO_EPSIZE-sizeof(TMC_MessageHeader_t), TMC_IO_EPSIZE);
	Endpoint_Write_Stream_LE_noUsbTask(MessageHeader, sizeof(TMC_MessageHeader_t));

	/* Indicate if the command has been aborted or not */
	return !IsTMCBulkINReset;
}



