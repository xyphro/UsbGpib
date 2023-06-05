/*
             LUFA Library
     Copyright (C) Dean Camera, 2019.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
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

#include "TestAndMeasurement.h"
#include "gpib.h"
#include <avr/eeprom.h>

#define LED(s) {if(s) PORTF |= (1<<5); else PORTF &= ~(1<<5);}

/** Contains the (usually static) capabilities of the TMC device. This table is requested by the
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
			},
		.Reserved2 = {0, 0, 0, 0, 0, 0},
		.bcdUSB488 = VERSION_BCD(1,0,0),
		.USB488IfCap1 =
			{
				.SupportTrigger         = 1, //0
				.SupportRenControl      = 1,
				.Is488IF                = 0,
				.Reserved				= 0,
			},
		.USB488IfCap2 =
			{
				.DT1Capable                 = 1,//0 => Device trigger no capability / full capability		=> send *TRG command
				.RL1Capable					= 1,//0 => Remote Local   no capability / full capability
				.SR1Capable					= 0,//0 => service request 
				.MandatorySCPI				= 0,//0
				.Reserved					= 0,//0
			},
		.Reserved3 = {0, 0, 0, 0, 0, 0, 0, 0},
	};

/** Current TMC control request that is being processed */
static uint8_t RequestInProgress = 0;

/** Stream callback abort flag for bulk IN data */
static bool IsTMCBulkINReset = false;

/** Stream callback abort flag for bulk OUT data */
static bool IsTMCBulkOUTReset = false;

/** Last used tag value for data transfers */
static uint8_t CurrentTransferTag = 0;

/** Length of last data transfer, for reporting to the host in case an in-progress transfer is aborted */
static uint16_t LastTransferLength = 0;

/** Buffer to hold the next message to sent to the TMC host */
static uint8_t NextResponseBuffer[64];

/** This will be set true after a indicator pulse command is received. If the next GPIB command starts with '!', a parameter has to be set */
static bool s_nextwrite_mightbeparameterset = false;


static uint32_t s_remaining_bytes_receive=0;

static uint8_t gpib_addr = 1;

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
int dbg = 0;

bool tmc_gpib_write_timedout(void)
{
	USB_USBTask();
	return IsTMCBulkOUTReset | IsTMCBulkINReset;
}

bool tmc_gpib_read_timedout(void)
{
	USB_USBTask();
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

static void TMC_SetInternalSerial(bool addGPIBAddress, uint8_t signatureLen);

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
			if ( (tmc_serial_string.UnicodeString[0] = 'H') &&
				 (tmc_serial_string.UnicodeString[1] = 'P') &&
				 (tmc_serial_string.UnicodeString[2] >= '0') &&
				 (tmc_serial_string.UnicodeString[2] <= '9')     )
			{
				hascomma = true;
			}
		
		
		if ((timeout_val == 0) || (len == 0)  || (!hascomma) ) /* timeout happened or length is 0 => build a serial number based on GPIB address */
		{
			TMC_SetInternalSerial(true, 20);
			gotStringViaGPIB = false;
		}
	}
	else
	{ /* no gpib address found => use normal serial number */
		TMC_SetInternalSerial(true, 20);
		gotStringViaGPIB = false;
	}

	gpib_ren(false);
	_delay_ms(100);
	gpib_ren(true);
	return gotStringViaGPIB;
}




static void TMC_SetInternalSerial(bool addGPIBAddress, uint8_t signatureLen) /* max signatureLen = 20 */
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
		if (signatureLen)
		{
			tmc_serial_string.UnicodeString[len++] = cpu_to_le16('_');
		}
	}
	for (uint8_t SerialCharNum = 0; SerialCharNum < signatureLen; SerialCharNum++)
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

void check_bootloaderEntry(void)
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
	int8_t startupCntHalfSec = -2;

	//mcusr_mirror = MCUSR; 
	MCUSR = 0; 
	wdt_disable(); 
	
	PORTB |=  (1<<2); /* PB2 = PULLUP */
	DDRB  &= ~(1<<2); /* PB2 = input*/
	
	SetupHardware();
	
	gpib_init();
	
	/* apply settings from eeprom */
	eeprom_busy_wait();	
	gpib_set_readtermination(eeprom_read_byte(105));
	
	
	//LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
	GlobalInterruptEnable();
	
	/* stop here until a gpib device is connected (checked in Timer0 ISR) */
	while (!gpib_is_connected())
	{
		_delay_ms(250);
		LED(1);
		_delay_ms(250);
		LED(0);
		check_bootloaderEntry();
		if (startupCntHalfSec != 0)
		{
			startupCntHalfSec++;
		}
	}

	/* if the instrument has been just powered-on */
	if(startupCntHalfSec==0)
	{
		startupCntHalfSec=eeprom_read_byte(106);
		if(startupCntHalfSec > 0)
		{
			/* wait here until complete power-up */
			while((startupCntHalfSec--) != 0)
			{
				_delay_ms(100);
				LED(1);
				_delay_ms(400);
				LED(0);
			}
		}
	}
	
	/* physically GPIB is connected, now check if any GPIB address is responsive */
	while (!findGpibdevice())
	{
		_delay_ms(100);
		LED(1);
		_delay_ms(100);
		LED(0);
		if (!gpib_is_connected()) /* we want to reset here if the device is unplugged */
		{
			LED(0);
			_delay_ms(500);
			wdt_enable(WDTO_250MS);	
			while (1);
		}
		check_bootloaderEntry();
	}; /* Identify the GPIB Address of the connected GPIB device */
	
	eeprom_busy_wait();
	/* found a responsive GPIB address */
	uint8_t ee_visa_name = eeprom_read_byte(104);
	if (ee_visa_name == 0x00 || ee_visa_name == 0xFF) /* setup USB descriptor with *IDN? or ID? command response */
	{
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
		if (eeprom_read_byte(104) == 0x01) /* use GPIB_NN_SSSSSSSSSSSSSSSSSSSS as descriptor */
		{
			TMC_SetInternalSerial(true, 20);
		}
		else if (eeprom_read_byte(104) == 0x02) /* use GPIB_NN_SSSSSS as descriptor */
		{
			TMC_SetInternalSerial(true, 6);
		}
		else if (eeprom_read_byte(104) == 0x03) /* use GPIB_NN as descriptor */
		{
			TMC_SetInternalSerial(true, 0);
		}
		else if (eeprom_read_byte(104) == 0x04) /* use SSSSSSSSSSSS as descriptor */
		{
			TMC_SetInternalSerial(false, 12);
		}
		gpib_ren(false);
		_delay_ms(100);
		gpib_ren(true);
	}

	/* all fine, now kickoff connect to USB to be able to communicate! */
	LED(1);
	USB_Attach();
	
	for (;;)
	{
		TMC_Task();
		
		check_bootloaderEntry();
		
		if (!gpib_is_connected()) /* check, if gpib is disconnected */
		{ /* when we get here, reset the MCU and disconnect from USB ! It will reconnect once plugged in to GPIB again */
			LED(0);
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
	//LEDs_Init();
	USB_Init();
	USB_Detach();

	
	/* update the TMC default serial number*/
	TMC_SetInternalSerial(false, 20);
	
	/* LED to output and turn on */
	DDRF |= (1<<5);
	LED(1);
	
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
	ConfigSuccess &= Endpoint_ConfigureEndpoint(TMC_NOTIFICATION_EPADDR, EP_TYPE_INTERRUPT, TMC_IO_EPSIZE, 1);
	ConfigSuccess &= Endpoint_ConfigureEndpoint(TMC_IN_EPADDR,  EP_TYPE_BULK, TMC_IO_EPSIZE, 1);
	ConfigSuccess &= Endpoint_ConfigureEndpoint(TMC_OUT_EPADDR, EP_TYPE_BULK, TMC_IO_EPSIZE, 1);
}

void TMC_resetstates(void);

/** Event handler for the USB_ControlRequest event. This is used to catch and process control requests sent to
 *  the device from the USB host before passing along unhandled control requests to the library for processing
 *  internally.
 */
void EVENT_USB_Device_ControlRequest(void)
{
	uint8_t TMCRequestStatus = TMC_STATUS_SUCCESS;
	uint8_t btag, statusReg;
	
	
	if ( ((USB_ControlRequest.wIndex == INTERFACE_ID_TestAndMeasurement) && ((USB_ControlRequest.bmRequestType & REQREC_INTERFACE)!=0)) ||
	     (((USB_ControlRequest.wIndex == TMC_IN_EPADDR) || (USB_ControlRequest.wIndex == TMC_OUT_EPADDR)) && ((USB_ControlRequest.bmRequestType & REQREC_ENDPOINT)!=0))     )
	{
		/* Process TMC specific control requests */
		switch (USB_ControlRequest.bRequest)
		{
			case Req_ReadStatusByte:
			
//Jump_To_Bootloader();			
				btag = USB_ControlRequest.wValue;

				timeout_start(50000); /* 0.5s timeout*/
				statusReg =  gpib_readStatusByte(gpib_addr, is_timedout);
				Endpoint_ClearSETUP();
				
				/* Write the request response byte */
				Endpoint_Write_8(TMC_STATUS_SUCCESS);
				Endpoint_Write_8(btag);
				Endpoint_Write_8(statusReg);

				Endpoint_ClearIN();
				Endpoint_ClearStatusStage();
				
/*Endpoint_SelectEndpoint(TMC_NOTIFICATION_EPADDR);
Endpoint_Write_8(TMC_STATUS_SUCCESS);
Endpoint_Write_8(btag);
Endpoint_Write_8(statusReg);
Endpoint_ClearIN();*/

	
				
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
					
//Endpoint_ResetEndpoint(TMC_IN_EPADDR);
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

#if 0					
					/* KG: Added for proper synchronsity handling */
					Endpoint_ResetEndpoint(TMC_IN_EPADDR);
					TMC_resetstates();
#endif
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

						/* Save the split request for later checking when a new request is received */
						RequestInProgress = Req_InitiateClear;
					}

					Endpoint_ClearSETUP();

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
					else if (IsTMCBulkINReset || IsTMCBulkOUTReset)
						TMCRequestStatus = TMC_STATUS_PENDING;
					else
						RequestInProgress = 0;

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
				
				LED(0);
				_delay_ms(250);
				LED(1);
				
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
				timeout_start(50000); /* 0.5s timeout*/
				gpib_localLockout(is_timedout);
				
				Endpoint_ClearSETUP();
				/* USBTMC Status response (1 Byte) */
				Endpoint_Write_8(TMC_STATUS_SUCCESS);
				Endpoint_ClearIN();
				Endpoint_ClearStatusStage();
				break;
			case Req_GoToLocal:
				timeout_start(50000); /* 0.5s timeout*/
				gpib_gotoLocal(gpib_addr, is_timedout);
				
				Endpoint_ClearSETUP();
				/* USBTMC Status response (1 Byte) */
				Endpoint_Write_8(TMC_STATUS_SUCCESS);
				Endpoint_ClearIN();
				Endpoint_ClearStatusStage();
				break;
		}
	}
}

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
/*
Process an internal command. This is triggered, if a indicator pulse command was received, followed
by a write of a command starting with an exclamation mark (!).
Syntax:
!XXYY
XX = command (hex)
				00 for VISA resource name:
					YY selects the method:
					0x00 or 0xff => Fully automatic (also try to sense ID? or *IDN? string and use it as serial number string)
					0x01         => Only detect GPIB address automatically and use it together with the 20 chars MCU signature as serial number string
					0x02         => Only detect GPIB address automatically and use it together with 6 chars MCU signature as serial number string
					0x03         => Only detect GPIB address automatically and use it as serial number string
					0x04         => Serial number string is only composed by 12 chars of the MCU signature
				01 for Termination method for READs:
					YY selects the method:
					0x00 or 0xff => EOI termination
					0x01         => EOI or '\n' (LF = linefeed)
					0x02         => EOI or '\r' (CR = carriage return)
				02 for startup delay (only when device is powered-on after the GPIBUSB):
					YY is the delay in half seconds.
					Max value = 120 == 60 sec. Set to 0 or 0xff to disable.
*/
void ProcessInternalCommand(uint8_t* const Data, uint8_t Length)
{
	uint8_t xx, yy;
	
	xx = charToval(Data[1])*16 + charToval(Data[2]);
	yy = charToval(Data[3])*16 + charToval(Data[4]);
	
	switch (xx)
	{
		case 0x00: /* automatic detection y */
			eeprom_update_if_changed(104, yy);
			break;
		case 0x01: /* select termination method */
			switch (yy)
			{
				case 0x01: /* \n */
					eeprom_update_if_changed(105, '\n');
					gpib_set_readtermination('\n');
					break;
				case 0x02: /* \r */
					eeprom_update_if_changed(105, '\r');
					gpib_set_readtermination('\r');
					break;
				default:
					eeprom_update_if_changed(105, '\0');
					gpib_set_readtermination('\0');
					break;
			}
			break;
		case 0x02: /* power-on delay */
			if (yy <= 120 || yy == 0xFF)
			{
				eeprom_update_if_changed(106, yy);
			}
			break;
	}
}

void ProcessSentMessage(uint8_t* const Data, uint8_t Length, bool isFirstTransfer, bool isLastTransfer, gpibtimeout_t ptimeoutfunc)
{
	uint8_t i, dat;
	bool timedout, isinternalcommand;
	
	
	/* check, if this is an internal command */ 
	isinternalcommand = isFirstTransfer && isFirstTransfer && s_nextwrite_mightbeparameterset && (Data[0] == '!');
	if (isinternalcommand)
	{
		ProcessInternalCommand(Data, Length);
	}
	else
	{
		timedout = false;
		
		gpib_ren(1); /* ensure that remote control is enabled */
		
		LED(0);
		if (isFirstTransfer)
			timedout = gpib_make_listener(gpib_addr, ptimeoutfunc);
			
		i = 0;
		while ( (Length > 0) && !timedout)
		{
			Length--;
			dat = Data[i++];
			timedout = gpib_writedat(dat, (Length == 0)  && isLastTransfer, ptimeoutfunc);
		}
		
		if (isLastTransfer && !timedout) /* in case of timeout the interface is cleared within the writedat function, no need to untalk!*/
			gpib_untalk_unlisten(ptimeoutfunc);
		LED(1);
	}
	s_nextwrite_mightbeparameterset = false;
}

uint8_t GetNextMessage(uint8_t* const Data, uint8_t maxlen, bool isFirstMessage, bool *pisLastMessage, gpibtimeout_t ptimeoutfunc)
{
	uint8_t c, i;
	bool    Eoi, timedout;
	
	gpib_ren(1); /* ensure that remote control is enabled */
	
	LED(0);	
	
	timedout = false;
	if (isFirstMessage)
		timedout = gpib_make_talker(gpib_addr, ptimeoutfunc);

	i = 0;
	Eoi = false;

	while (!Eoi && (i < maxlen) && !timedout)
	{
	
		c = gpib_readdat(&Eoi, &timedout, ptimeoutfunc); 
		if (!timedout)
			NextResponseBuffer[i++] = c;
	}
		
	if (Eoi && !timedout) /* in case of timeout, no need to unlisten => interface clear done in readdat function! */
		gpib_untalk_unlisten(ptimeoutfunc);

	if (timedout) /* in case of timedout, simulate an end of message */
		Eoi = true;
	*pisLastMessage = Eoi;
	
//NextResponseBuffer[i++]	= gpib_search();

	memcpy((char*)Data, (char*)NextResponseBuffer, i);
	
	LED(1);

	return i;
}

bool TMC_LastMessageComplete = true;
bool TMC_eom;
bool TMC_InLastMessageComplete = true;

void TMC_resetstates(void)
{
	TMC_LastMessageComplete = true;
	TMC_InLastMessageComplete = true;
	s_remaining_bytes_receive = 0;
	gpib_interface_clear();
//	gpib_untalk_unlisten();
}

/** Function to manage TMC data transmission and reception to and from the host. */
void TMC_Task(void)
{
	bool lastmessage;
	/* Device must be connected and configured for the task to run */
	if (USB_DeviceState != DEVICE_STATE_Configured)
	  return;

	TMC_MessageHeader_t MessageHeader;
	uint8_t             MessagePayload[128], curlen;
	
	

	if (s_remaining_bytes_receive == 0)
	{
	
		/* Try to read in a TMC message from the interface, process if one is available */
		if (ReadTMCHeader(&MessageHeader))
		{
		dbg++;
		
			/* Indicate busy */
			//LEDs_SetAllLEDs(LEDMASK_USB_BUSY);

			switch (MessageHeader.MessageID)
			{
				case TMC_MESSAGEID_TRIGGER:
					timeout_start(50000); /* 0.5s timeout*/
					gpib_trigger(gpib_addr, is_timedout);
					Endpoint_ClearOUT();					
					break;
				case TMC_MESSAGEID_DEV_DEP_MSG_OUT:
					s_remaining_bytes_receive = MessageHeader.TransferSize;
					
					LastTransferLength = 0;
					curlen = MIN(TMC_IO_EPSIZE-sizeof(TMC_MessageHeader_t), MessageHeader.TransferSize);
					//
					while (Endpoint_Read_Stream_LE(MessagePayload, curlen, &LastTransferLength) ==
						   ENDPOINT_RWSTREAM_IncompleteTransfer)
					{
						if (IsTMCBulkOUTReset)
						  break;
					}					
					
					s_remaining_bytes_receive -= curlen;
					
					TMC_eom = (MessageHeader.MessageIDSpecific.DeviceOUT.LastMessageTransaction != 0);
					lastmessage =  TMC_eom && (s_remaining_bytes_receive==0);
					ProcessSentMessage(MessagePayload, curlen, TMC_LastMessageComplete, lastmessage, tmc_gpib_write_timedout);
					
					/* Select the Data Out endpoint, this has to be done because the timeout function cal select the control endpoint */
					Endpoint_SelectEndpoint(TMC_OUT_EPADDR);
					Endpoint_ClearOUT();
					
					TMC_LastMessageComplete = lastmessage;
					break;
				case TMC_MESSAGEID_DEV_DEP_MSG_IN:
					Endpoint_ClearOUT();
//FIXME: ZLP not OK! -------------------------------------------------------v
					curlen = MIN(TMC_IO_EPSIZE-sizeof(TMC_MessageHeader_t) -1, MessageHeader.TransferSize);
					MessageHeader.TransferSize = GetNextMessage(MessagePayload, curlen, TMC_InLastMessageComplete, &lastmessage, tmc_gpib_read_timedout);
					TMC_InLastMessageComplete = lastmessage;
					
					MessageHeader.MessageIDSpecific.DeviceOUT.LastMessageTransaction = lastmessage;
					if (!IsTMCBulkINReset)
						WriteTMCHeader(&MessageHeader);					
					
					LastTransferLength = 0;
					if (!IsTMCBulkINReset)
					{
						while (Endpoint_Write_Stream_LE(MessagePayload, MessageHeader.TransferSize, &LastTransferLength) ==
							   ENDPOINT_RWSTREAM_IncompleteTransfer)
						{
							if (IsTMCBulkINReset)
							  break;
						}
					}
//TODO: short packet handling not OK!!!!! Commit an empty packet 
//					if (!IsTMCBulkINReset)

					/* Also in case of a timeout, the host does not expire a Bulk IN IRP, so we still need to commit an empty endpoint to retire the IRP */
					Endpoint_SelectEndpoint(TMC_IN_EPADDR);
					Endpoint_ClearIN();
					
					if (IsTMCBulkINReset)
					{
						//Endpoint_SelectEndpoint(TMC_IN_EPADDR);
						//Endpoint_AbortPendingIN();
						/* KG: Added for proper synchronsity handling */
						//Endpoint_ResetEndpoint(TMC_IN_EPADDR);
						TMC_resetstates();
					}

					
					break;
				default:
					Endpoint_StallTransaction();
					break;
			}

			//LEDs_SetAllLEDs(LEDMASK_USB_READY);
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
			
			//
			while (Endpoint_Read_Stream_LE(MessagePayload, curlen, &LastTransferLength) ==
				   ENDPOINT_RWSTREAM_IncompleteTransfer)
			{
				if (IsTMCBulkOUTReset)
				  break;
			}
			s_remaining_bytes_receive -= curlen;

			Endpoint_ClearOUT();
			
			lastmessage = TMC_eom && (s_remaining_bytes_receive==0);
			TMC_LastMessageComplete = lastmessage;
			ProcessSentMessage(MessagePayload, curlen, false, lastmessage, tmc_gpib_write_timedout);
		}
	}

	if (IsTMCBulkOUTReset || IsTMCBulkINReset)
		TMC_resetstates();
	
	/* All pending data has been processed - reset the data abort flags */
	IsTMCBulkINReset  = false;
	IsTMCBulkOUTReset = false;
}

/** Attempts to read in the TMC message header from the TMC interface.
 *
 *  \param[out] MessageHeader  Pointer to a location where the read header (if any) should be stored
 *
 *  \return Boolean \c true if a header was read, \c false otherwise
 */
bool ReadTMCHeader(TMC_MessageHeader_t* const MessageHeader)
{
	uint16_t BytesTransferred;
	uint8_t  ErrorCode;

	/* Select the Data Out endpoint */
	Endpoint_SelectEndpoint(TMC_OUT_EPADDR);

	/* Abort if no command has been sent from the host */
	if (!(Endpoint_IsOUTReceived()))
	  return false;

	/* Read in the header of the command from the host */
	BytesTransferred = 0;
	while ((ErrorCode = Endpoint_Read_Stream_LE(MessageHeader, sizeof(TMC_MessageHeader_t), &BytesTransferred)) ==
	       ENDPOINT_RWSTREAM_IncompleteTransfer)
	{
		if (IsTMCBulkOUTReset)
		  break;
	}

	/* Store the new command tag value for later use */
	CurrentTransferTag = MessageHeader->Tag;

	/* Indicate if the command has been aborted or not */
	return (!(IsTMCBulkOUTReset) && (ErrorCode == ENDPOINT_RWSTREAM_NoError));
}

bool WriteTMCHeader(TMC_MessageHeader_t* const MessageHeader)
{
	uint16_t BytesTransferred;
	uint8_t  ErrorCode;

	/* Set the message tag of the command header */
	MessageHeader->Tag        =  CurrentTransferTag;
	MessageHeader->InverseTag = ~CurrentTransferTag;

	/* Select the Data In endpoint */
	Endpoint_SelectEndpoint(TMC_IN_EPADDR);

	/* Send the command header to the host */
	BytesTransferred = 0;
	while ((ErrorCode = Endpoint_Write_Stream_LE(MessageHeader, sizeof(TMC_MessageHeader_t), &BytesTransferred)) ==
	       ENDPOINT_RWSTREAM_IncompleteTransfer)
	{
		if (IsTMCBulkINReset)
		  break;
	}

	/* Indicate if the command has been aborted or not */
	return (!(IsTMCBulkINReset) && (ErrorCode == ENDPOINT_RWSTREAM_NoError));
}




/*
HP 3457A: END,ALWAYS => turns on EOI signal!

ISSUES:
  - short packet handling not OK!!!!! Commit an empty packet   
  
BOOT UPDATE:
  V:
  cd V:\Data\Projekt\Privat\gpibadapter\lufa-master\Demos\Device\Incomplete\TestAndMeasurement
  copy TestAndMeasurement.bin G:\FLASH.BIN /Y
*/
