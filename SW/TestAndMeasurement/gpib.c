/*
GPIB to USB Adaptor
by Xyphro 2024
*/

/* 
version 2.0
Modified for version 2.0 hardware with two LEDs by D Conway October 2024
*/

#include "gpib.h"
#include "gpib_priv.h" 
#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "global.h"

/* Is a GPIB device connected?  */
#define GPIB_DEVICE_CONNECTSTATE_UNKNOWN      (0)
#define GPIB_DEVICE_CONNECTSTATE_DISCONNECTED (1)
#define GPIB_DEVICE_CONNECTSTATE_CONNECTED    (2)

/*  Defines code for version 2 hardware.
    Comment out the #define VER2 line to compile software for
    single red LED, version 1 hardware
*/
#define  VER2

  /* Turn LEDs ON/OFF by switching port Hi/Lo */
  #define LED_RED_ON  { PORTF |=  (1<<5); }
  #define LED_RED_OFF { PORTF &= ~(1<<5); }

  #define LED_GRN_ON  { PORTF |=  (1<<4); }
  #define LED_GRN_OFF { PORTF &= ~(1<<4); }
  #define LED_GRN_TGL { PORTF ^=  (1<<4); }   // Toggle Green LED 

/**********************************************************************************************************
 * STATIC functions
 **********************************************************************************************************/
static volatile uint8_t timer0_100mscounter;  /*Int Timer count */
volatile bool timer0_ticked = false; /* flag going high every 100ms */
static uint8_t  timer0_div;
static uint8_t  s_device_state = GPIB_DEVICE_CONNECTSTATE_UNKNOWN;
static uint8_t s_gpib_disconnect_counter;
static volatile bool     s_gpib_transaction_active = false; /* TRUE, if a device is addressed as talker or listener */

char s_terminator = '\0'; /* \0 = no termination character - EOI only, other options are '\n' or '\r' */
 
static void timer_init(void)
{
	TCCR0B = 5; // Prescaler 1024 = 15625 Hz
	// Enable overflow interrupt
	TIMSK0 |= (1<<TOIE0);
	timer0_div = 0;
	timer0_100mscounter = 0;
}

/* The timer tick controls the green LED (ver 2 only).
If the gpib device is inactive and connected then the green LED is on  to indicate device is connected
If the gpib device is inactive and disconnected then the green LED is off  to indicate nil device
If the gpib device is active then the green LED is toggled to indicate activity */
ISR (TIMER0_OVF_vect)
{
	timer0_div++;
	timer0_ticked = true;
	if (timer0_div >= 6) /* has 100ms passed? */
	{
		timer0_100mscounter++;
		timer0_div = 0;

		if (!s_gpib_transaction_active) /* only check, if no GPIB transaction is active */
		{
			if ( !(DDRF & (1<<6)) ) /* is ATN line tristated? */
			{
				if (!ATN_STATE ) /* is ATN LOW? This can only happen if no GPIB device is connected/powered */
				{
					if (s_gpib_disconnect_counter >= 2)
					{ /* after 100-200ms with ATN low, assume, that there is no GPIB device connected */
						s_device_state = GPIB_DEVICE_CONNECTSTATE_DISCONNECTED;
						#ifdef VER2
						   // Turn off GPIB LED green if no gpib device connected.
						   LED_GRN_OFF
						#endif
					}
					else
					{
						s_gpib_disconnect_counter++;
					}
				}
				else
				{ /* device is connected */
					s_gpib_disconnect_counter = 0;
					s_device_state = GPIB_DEVICE_CONNECTSTATE_CONNECTED;
					#ifdef VER2
					   // Turn on GPIB LED green if gpib device connected but not active
					   LED_GRN_ON
					#endif
				}
			}
			else
			{
				s_gpib_disconnect_counter = 0;

			}
		}
		else
		{
			#ifdef VER2
			  // if gpib is active, then toggle the green led at 5Hz to indicate activity.
			  LED_GRN_TGL
			#endif
		}
	}
}

void gpib_recover(void)
{
	gpib_init();
}

static bool gpib_tx(uint8_t dat, bool iscommand, gpibtimeout_t ptimeoutfunc)
{
	return gpib_tx_quick(dat, iscommand, ptimeoutfunc, true);
}

static bool gpib_cmd_LAG(uint8_t addr, gpibtimeout_t ptimeoutfunc)
{
	bool result;
	result = gpib_tx((addr & 0x1f) | 0x20, true, ptimeoutfunc);
	if (addr & 0xe0)
	{ /* send a secondary address? */
		result = gpib_tx(0x60, true, ptimeoutfunc);        // SAG (SA0)
	}
	return result;
}

static bool gpib_cmd_SECADDR(uint8_t addr, gpibtimeout_t ptimeoutfunc)
{
	return gpib_tx(addr | 0x60, true, ptimeoutfunc);
}

static bool gpib_cmd_TAG(uint8_t addr, gpibtimeout_t ptimeoutfunc)
{
	bool result;
	result = gpib_tx((addr & 0x1f) | 0x40, true, ptimeoutfunc);
	if (addr & 0xe0)
	{ /* send a secondary address? */
		result = gpib_tx(0x60, true, ptimeoutfunc);        // SAG (SA0)	
	}
	return result;
}

static bool gpib_cmd_UNL(gpibtimeout_t ptimeoutfunc)
{
	return gpib_tx(0x3F, true, ptimeoutfunc);
}

static bool gpib_cmd_UNT(gpibtimeout_t ptimeoutfunc)
{
	return gpib_tx(0x5F, true, ptimeoutfunc);
}

static bool gpib_cmd_LLO(gpibtimeout_t ptimeoutfunc) // local lockout
{
	return gpib_tx(0x11, true, ptimeoutfunc);
}

static bool gpib_cmd_GTL(gpibtimeout_t ptimeoutfunc) // goto local
{
	return gpib_tx(0x01, true, ptimeoutfunc);
}


static bool gpib_cmd_SPE(gpibtimeout_t ptimeoutfunc) // serial poll enable
{
	return gpib_tx(0x18, true, ptimeoutfunc);
}

static bool gpib_cmd_SPD(gpibtimeout_t ptimeoutfunc) // serial poll disable
{
	return gpib_tx(0x19, true, ptimeoutfunc);
}

static bool gpib_cmd_GET(gpibtimeout_t ptimeoutfunc) // group execute trigger (addressed command)
{
	return gpib_tx(0x08, true, ptimeoutfunc);
}

uint8_t gpib_readStatusByte(uint8_t addr, gpibtimeout_t ptimeoutfunc)
{
	bool timedout, eoi;
	uint8_t status;

	timedout = false;
	status = 0;

	s_gpib_transaction_active = true;
	if (!timedout)
		timedout = gpib_cmd_LAG(0, ptimeoutfunc);
	if (!timedout)
		timedout = gpib_cmd_SPE(ptimeoutfunc);
	if (!timedout)
		timedout = gpib_cmd_TAG(addr, ptimeoutfunc); 

	if (!timedout)
		status = gpib_readdat(&eoi, &timedout, ptimeoutfunc);

	if (!timedout)
		timedout = gpib_cmd_SPD(ptimeoutfunc);
	if (!timedout)
		timedout = gpib_cmd_UNT(ptimeoutfunc);
	if (timedout)
		gpib_recover();
	s_gpib_transaction_active = false;
	return status;
}

bool gpib_localLockout(gpibtimeout_t ptimeoutfunc)
{
	bool timedout;

	timedout = gpib_cmd_LLO(ptimeoutfunc);
	if (timedout)
		gpib_recover();
	return timedout;
}

bool gpib_gotoLocal(uint8_t addr, gpibtimeout_t ptimeoutfunc)
{
	bool timedout;

	timedout = gpib_cmd_LAG(addr, ptimeoutfunc);
	if (!timedout)
		timedout = gpib_cmd_GTL(ptimeoutfunc);

	_delay_ms(100);	// this is not done by the NIUSB adapter, but seems to be required for measurement devices using NI chipset

	if (!timedout)
		timedout = gpib_cmd_UNL(ptimeoutfunc);

	//ATN_HIGH;
	//_delay_us(14);

	if (timedout)
		gpib_recover();
	return timedout;
}


bool gpib_trigger(uint8_t addr, gpibtimeout_t ptimeoutfunc)
{
	bool timedout;

	timedout = gpib_cmd_LAG(addr, ptimeoutfunc); 
	if (!timedout)
		timedout = gpib_cmd_GET(ptimeoutfunc);
	if (!timedout)
		timedout = gpib_cmd_UNL(ptimeoutfunc);

	if (timedout)
		gpib_recover();
	return timedout;
}


/*
static bool gpib_cmd_DCL(gpibtimeout_t ptimeoutfunc) // device clear
{
	return gpib_tx(0x14, true, ptimeoutfunc);
}
*/

static bool gpib_cmd_SDC(gpibtimeout_t ptimeoutfunc) // selective device clear
{
	return gpib_tx(0x04, true, ptimeoutfunc);
}


bool gpib_sdc(uint8_t addr, gpibtimeout_t ptimeoutfunc)
{
	bool timedout;

	s_gpib_transaction_active = true;
	timedout = gpib_cmd_LAG(addr, ptimeoutfunc); 
	if (!timedout)
		timedout = gpib_cmd_SDC(ptimeoutfunc);
	if (!timedout)
		timedout = gpib_cmd_UNL(ptimeoutfunc);
	if (timedout)
		gpib_recover();
	s_gpib_transaction_active = false;

	_delay_ms(150); // this is not done by the NI USB adapter, but seems to be required for products using NI USB adapter!
	return timedout;
}


/**********************************************************************************************************
 * global API functions
 **********************************************************************************************************/

void gpib_init(void)
{
	// PB5 = REN
	DDRD  = 0x00;
	PORTD = 0x00;
	PORTB &= ~((1<<4) | (1<<5) | (1<<6));
	DDRB  = (DDRB & (~((1<<4) | (1<<6)))) | (1<<5);
	PORTC &= ~((1<<6) | (1<<7));
	DDRC  &= ~((1<<6) | (1<<7));
	PORTE &= ~(1<<2);
	DDRE  &= ~(1<<2);
	PORTF &= ~((1<<6) | (1<<7));
	DDRF  &= ~((1<<6) | (1<<7));


	s_gpib_transaction_active = false;
	s_gpib_disconnect_counter = 0;

	gpib_interface_clear();
	timer_init(); /* init timeout timer */
}

bool gpib_is_connected(void)
{
	if ( (DDRF & (1<<6)) != 0 )
	{
		if (!s_gpib_transaction_active) /* only check, if no GPIB transaction is active */
		{
			_delay_us(10);
			ATN_HIGH;
			_delay_us(10);
		}
		else
		{
			return true;
		}

		//return s_device_state == GPIB_DEVICE_CONNECTSTATE_CONNECTED;
	}
	else
	{
		//return s_device_state == GPIB_DEVICE_CONNECTSTATE_CONNECTED;
	}
	return !!ATN_STATE; /* is ATN LOW? This can only happen if no GPIB device is connected/powered */
}

void gpib_ren(bool enable)
{
	if (enable)
	{
		REN_LOW; /* remote enable */
	}
	else
	{
		REN_HIGH; /* remote disable */
	}
	//s_gpib_transaction_active = false;
}




void gpib_interface_clear(void)
{
	NRFD_HIGH;
	IFC_LOW; /* interface clear */
	_delay_us(120);
	IFC_HIGH; /* interface clear */
	_delay_ms(10);
	s_gpib_transaction_active = false;
}




uint8_t gpib_readdat(bool *pEoi, bool *ptimedout, gpibtimeout_t ptimeoutfunc)
{
	return gpib_readdat_quick(pEoi, ptimedout, ptimeoutfunc, true);
};

bool gpib_untalk_unlisten(gpibtimeout_t ptimeoutfunc)
{
	bool timedout;
	timedout = gpib_cmd_UNL(ptimeoutfunc);
	if (!timedout)
		timedout = gpib_cmd_UNT(ptimeoutfunc);
	if (timedout)
		gpib_recover();

	s_gpib_transaction_active = false;
	return timedout;
}

bool  gpib_make_talker(uint8_t addr, gpibtimeout_t ptimeoutfunc)
{
	bool timedout;

	s_gpib_transaction_active = true;

	timedout = gpib_cmd_UNL(ptimeoutfunc);
	if (!timedout)
		timedout = gpib_cmd_LAG(0, ptimeoutfunc); /* signal that controller is listener */
	if (!timedout)
		timedout = gpib_cmd_TAG(addr, ptimeoutfunc); /* address as talker*/
	//_delay_us(10);

	NDAC_LOW;   /* make NDAC L */
	//NRFD_LOW;

	if (timedout)
		gpib_recover();
	return timedout;
}


bool gpib_make_listener(uint8_t addr, gpibtimeout_t ptimeoutfunc)
{
	bool timedout;
	s_gpib_transaction_active = true;
	timedout = gpib_cmd_UNT(ptimeoutfunc);
	if (!timedout)
		timedout = gpib_cmd_TAG(0, ptimeoutfunc); /* signal that controller is talker */
	if (!timedout)
		timedout = gpib_cmd_UNL(ptimeoutfunc);
	if (!timedout)
		timedout = gpib_cmd_LAG(addr, ptimeoutfunc); /* address target as listener*/

	if (timedout)
		gpib_recover();
	return timedout;
}

bool gpib_writedat(uint8_t dat, bool Eoi, gpibtimeout_t ptimeoutfunc)
{
	return gpib_writedat_quick(dat, Eoi, ptimeoutfunc, true);
}

char gpib_get_readtermination(void)
{
	return s_terminator;
}

void gpib_set_readtermination(char terminator)
{
	switch(terminator)
	{
		case '\n':
			s_terminator = '\n';
			break;
		case '\r':
			s_terminator = '\r';
			break;
		default:
			s_terminator = '\0';
			break;
	}
}


static uint16_t timeout_val;

static void timeout_start(uint16_t timeout)
{
	timeout_val = timeout;
}

static bool is_timedout(void)
{
	_delay_us(10);
	if (timeout_val == 0)
		return true;

	timeout_val--;
	return false;
}

uint8_t gpib_search(void)
{
	uint8_t addr, foundaddr;

	s_gpib_transaction_active = true;
	timeout_start(500);
	gpib_tx(0x3F, true, is_timedout); // UNL

	foundaddr = 255;
	addr = 0; // start searching from GPIB Address 1 onwards as 0 is reserved for controller
	do
	{
		addr++;
		if ((addr & 0x1f) != 31)
		{
			timeout_start(500);
			gpib_cmd_LAG(addr, is_timedout);

			ATN_HIGH; /* make ATN H */
			_delay_ms(2);
			if ( (NDAC_STATE == 0) && (ATN_STATE != 0))
			{
				foundaddr = addr;
			}
		}

	}
	while ( (addr < 63) && (foundaddr == 255));

	timeout_start(500);
	gpib_tx(0x3F, true, is_timedout); // UNL


	/* if the device needs a secondary address, ensure, that it really cannot be addressed without secondary address */
	if (addr >= 32)
	{
		/* address once without SA. If it responds, force it to this primary addressing only! */
		timeout_start(500);
		gpib_cmd_LAG(addr & 0x1f, is_timedout);
		ATN_HIGH; /* make ATN H */
		_delay_ms(2);
		if ( (NDAC_STATE == 0) && (ATN_STATE != 0))
		{
			foundaddr = addr & 0x1f;
		}
		timeout_start(500);
		gpib_tx(0x3F, true, is_timedout); // UNL
	}

	s_gpib_transaction_active = false;

	return foundaddr;
}
