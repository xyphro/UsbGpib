#ifndef GPIB_PRIV_H_
#define GPIB_PRIV_H_

#include <avr/io.h>
#include <util/delay.h>


#define ATN_LOW   DDRF |=  (1<<6)
#define ATN_HIGH  DDRF &= ~(1<<6)
#define NDAC_LOW  DDRC |=  (1<<7)
#define NDAC_HIGH DDRC &= ~(1<<7)
#define NRFD_LOW  DDRC |=  (1<<6)
#define NRFD_HIGH DDRC &= ~(1<<6)
#define DAV_LOW   DDRB |=  (1<<6)
#define DAV_HIGH  DDRB &= ~(1<<6)
#define EOI_LOW   DDRB |=  (1<<4)
#define EOI_HIGH  DDRB &= ~(1<<4)
#define REN_LOW   DDRB |=  (1<<5)
#define REN_HIGH  DDRB &= ~(1<<5)

#define IFC_LOW   DDRE |=  (1<<2)
#define IFC_HIGH  DDRE &= ~(1<<2)

#define DATA_DIR_INPUT  DDRD = 0x00
#define DATA_DIR_OUTPUT DDRD = 0xff

#define DAV_STATE  (PINB & (1<<6))
#define NDAC_STATE (PINC & (1<<7))
#define NRFD_STATE (PINC & (1<<6))
#define ATN_STATE  (PINF & (1<<6))
#define EOI_STATE  (PINB & (1<<4))
#define SRQ_STATE  (PINF & (1<<7))

#define ATN_OUT_STATE ((DDRF &  (1<<6))==0)

void gpib_recover(void);
extern char s_terminator; /* defined in gpib.c */
extern volatile bool timer0_ticked; /* flag going high every 100ms - defined in gpib.c */

/* check if 100ms timer ticked. returns true once every 100ms */
static inline bool timer_ticked(void)
{
	bool ticked = timer0_ticked;
	timer0_ticked = false;
	return ticked;
}



static inline bool gpib_tx_quick(uint8_t dat, bool iscommand, gpibtimeout_t ptimeoutfunc, bool quickTimeoutPoll)
{
	bool timedout = false;

	
	if (iscommand)
	{ /* set or reset ATN line. It based on a ticket and these delay times are important. The timing was measured on a commercial GPIB adapter. */
		bool atn_was_high = ATN_OUT_STATE;
		if (atn_was_high)
			_delay_us(220);
		ATN_LOW;
		if (atn_was_high)
			_delay_us(70);
	}
	else
	{
		bool atn_was_low = !ATN_OUT_STATE;
		if (atn_was_low)
			_delay_us(220);
		ATN_HIGH;
		if (atn_was_low)
			_delay_us(70);
	}
		
	DAV_HIGH;
	
	
	NRFD_HIGH;
	NDAC_HIGH;  /* they should be already high, but let's enforce it */
	

	DDRD = dat;
	
	//_delay_us(10);
		
	/* wait until ready for data acceptance (NRFD=H, NDAC=L)*/
	do
	{
		if (timer_ticked() || quickTimeoutPoll) /* the timeout function can consume significant time */
			timedout = ptimeoutfunc();
	}
	while ( (NRFD_STATE == 0) && !timedout); /* wait until ready for data (NRFD to get high) */

	if (!timedout)
	{
		DAV_LOW;
		do
		{
			if (timer_ticked() || quickTimeoutPoll)
				timedout = ptimeoutfunc();
		}
		while ( (NDAC_STATE == 0) && !timedout ); /* wait until NDAC gets high*/
		DAV_HIGH;
	}
	
	DDRD = 0x00; /* release data bus */
	
	if (timedout)
	{
		gpib_recover();
	}

	return timedout;
}

static inline bool gpib_writedat_quick(uint8_t dat, bool Eoi, gpibtimeout_t ptimeoutfunc, bool quickTimeoutPoll)
{
	bool timedout;
	if (Eoi)
	{
		EOI_LOW; /* make EOI L */
	}
	timedout = gpib_tx_quick(dat, false, ptimeoutfunc, quickTimeoutPoll);
	EOI_HIGH;    /* make EOI H */
	return timedout;
}

static inline uint8_t gpib_readdat_quick(bool *pEoi, bool *ptimedout, gpibtimeout_t ptimeoutfunc, bool quickTimeoutPoll)
{
	uint8_t c;
	bool eoi, timedout;	
	c = 0;
	eoi = false;	

	timedout = false;
	
	// ensure that ATN is high
	bool atn_was_low = !ATN_OUT_STATE;
	if (atn_was_low)
		_delay_us(220);
	ATN_HIGH;
	
	/* skipping NRFD LOW step, because we are able to handshake and response to data */
	NDAC_LOW;
	NRFD_HIGH;
	
	if (atn_was_low)
		_delay_us(70);
	
	
	do
	{
		if (timer_ticked() || quickTimeoutPoll)
			timedout = ptimeoutfunc();
	}
	while ( (DAV_STATE != 0) && !timedout ); /* wait until DAV gets low */
	
	if (!timedout)
	{
		NRFD_LOW;
		c = ~PIND;
		eoi = (EOI_STATE == 0) ;
		NDAC_HIGH;
		
		do
		{
			if (timer_ticked() || quickTimeoutPoll)
				timedout = ptimeoutfunc();
		}
		while ( (DAV_STATE == 0) && !timedout ); /* wait until DAV gets high */
	}

	
	if (s_terminator == '\0')
		*pEoi = eoi;
	else
		*pEoi = eoi || (c == s_terminator);

	if (timedout)
	{
		gpib_recover();
	}
	*ptimedout = timedout;
	
	NRFD_LOW;
	
	return c;
};

static bool gpib_is_srq_active(void)
{
	return !SRQ_STATE;
}




#endif /* GPIB_PRIV_H_ */
