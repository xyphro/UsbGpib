
#include "miniparser.h"
#include <avr/pgmspace.h>

#include "TestAndMeasurement.h"
#include "gpib.h"

typedef void(*parser_func_t) (void);

#define PARSER_ENTRYBYTECOUNT 1

/* Below blob is generated using a selfmade python script to convert a command list into a
 * hierachical parser tree for memory size efficient parsing of commands.
 * Yes, this is pretty unreadable, but: code and speed efficient :-)
 */
static const char cmd_parser[] PROGMEM = {0x06, 'a' , 0x0d, 't' , 0x3f, 'v' , 0x6e, '0' , 
    0x77, 'r' , 0x8e, 's' , 0x9a, 0x02, 'u' , 0x12, 'd' , 0x39, 0x01, 't' , 0x15, 0x01, 
    'o' , 0x18, 0x01, 'i' , 0x1b, 0x01, 'd' , 0x1e, 0x02, ' ' , 0x23, '?'|0x80 , 0x03, 
    0x02, 'o' , 0x28, 's' , 0x30, 0x02, 'n'|0x80 , 0x00, 'f' , 0x2d, 0x01, 'f'|0x80 , 
    0x01, 0x01, 'l' , 0x33, 0x01, 'o' , 0x36, 0x01, 'w'|0x80 , 0x02, 0x01, 'd' , 0x3c, 
    0x01, 'r'|0x80 , 0x0f, 0x01, 'e' , 0x42, 0x01, 'r' , 0x45, 0x01, 'm' , 0x48, 0x02, 
    ' ' , 0x4d, '?'|0x80 , 0x07, 0x04, 'c' , 0x56, 'l' , 0x59, 'e' , 0x5c, 's' , 0x62, 
    0x01, 'r'|0x80 , 0x04, 0x01, 'f'|0x80 , 0x05, 0x01, 'o' , 0x5f, 0x01, 'i'|0x80 , 
    0x06, 0x01, 't' , 0x65, 0x01, 'o' , 0x68, 0x01, 'r' , 0x6b, 0x01, 'e'|0x80 , 0x08, 
    0x01, 'e' , 0x71, 0x01, 'r' , 0x74, 0x01, '?'|0x80 , 0x09, 0x02, '0' , 0x7c, '1' , 
    0x84, 0x01, '0' , 0x7f, 0x02, '0'|0x80 , 0x0a, '1'|0x80 , 0x0b, 0x01, '0' , 0x87, 
    0x03, '0'|0x80 , 0x0c, '1'|0x80 , 0x0d, '2'|0x80 , 0x0e, 0x01, 'e' , 0x91, 0x01, 
    's' , 0x94, 0x01, 'e' , 0x97, 0x01, 't'|0x80 , 0x10, 0x01, 't' , 0x9d, 0x01, 'r' , 
    0xa0, 0x01, 'i' , 0xa3, 0x01, 'n' , 0xa6, 0x01, 'g' , 0xa9, 0x02, ' ' , 0xae, '?'|0x80 , 
    0x13, 0x02, 's' , 0xb3, 'n' , 0xbf, 0x01, 'h' , 0xb6, 0x01, 'o' , 0xb9, 0x01, 'r' , 
    0xbc, 0x01, 't'|0x80 , 0x11, 0x01, 'o' , 0xc2, 0x01, 'r' , 0xc5, 0x01, 'm' , 0xc8, 
    0x01, 'a' , 0xcb, 0x01, 'l'|0x80 , 0x12 };

void cmd_autoid_on(void);
void cmd_autoid_off(void);
void cmd_autoid_slow(void);
void cmd_autoid_query(void);
void cmd_term_cr(void);
void cmd_term_lf(void);
void cmd_term_eoi(void);
void cmd_term_query(void);
void cmd_term_store(void);
void cmd_ver_query(void);
void cmd_0000(void);
void cmd_0001(void);
void cmd_0100(void);
void cmd_0101(void);
void cmd_0102(void);
void cmd_addr(void);
void cmd_reset(void);
void cmd_string_short(void);
void cmd_string_normal(void);
void cmd_string_query(void);

static const parser_func_t cmd_list[] PROGMEM = {
    cmd_autoid_on, 
    cmd_autoid_off, 
    cmd_autoid_slow, 
    cmd_autoid_query, 
    cmd_term_cr, 
    cmd_term_lf, 
    cmd_term_eoi, 
    cmd_term_query, 
    cmd_term_store, 
    cmd_ver_query, 
    cmd_0000, 
    cmd_0001, 
    cmd_0100, 
    cmd_0101, 
    cmd_0102, 
    cmd_addr, 
    cmd_reset, 
    cmd_string_short, 
    cmd_string_normal, 
    cmd_string_query };


void cmd_autoid_on(void)
{
	eeprom_update_if_changed(104, 0x00); // autoId ON
}

void cmd_autoid_off(void)
{
	eeprom_update_if_changed(104, 0x01); // autoId OFF
}

void cmd_autoid_slow(void)
{
	eeprom_update_if_changed(104, 0x02); // autoId SLOW
}

void cmd_autoid_slower(void)
{
	eeprom_update_if_changed(104, 0x03); // autoId SLOWER
}

void cmd_autoid_slowest(void)
{
	eeprom_update_if_changed(104, 0x04); // autoId SLOWEST
}

void cmd_autoid_query(void)
{
	switch(eeprom_read_byte((uint8_t*)104))
	{
		case 0x04:
		case 0x03:
		case 0x02:
			set_internal_response((void*)"slow", 4);
			break;
		case 0x01:
			set_internal_response((void*) "off", 3);
			break;
		default: // 0x00 or 0xff
			set_internal_response((void*)  "on", 2);
			break;
	}
}

void cmd_term_cr(void)
{
	gpib_set_readtermination('\r');
}

void cmd_term_lf(void)
{
	gpib_set_readtermination('\n');
}

void cmd_term_eoi(void)
{
	gpib_set_readtermination('\0');
}

void cmd_term_query(void)
{
	char t = gpib_get_readtermination();
	switch (t)
	{
		case '\r':	set_internal_response((void*) "cr", 2); break;
		case '\n':	set_internal_response((void*) "lf", 2); break;
		default:    set_internal_response((void*)"eoi", 3); break;
	}
}

void cmd_term_store(void)
{
	eeprom_update_if_changed(105, gpib_get_readtermination());
}

void cmd_ver_query(void)
{
	set_internal_response((void*)"V1.9", 4);
}

void cmd_0000(void)
{
	eeprom_update_if_changed(104, 0x00); // autoId ON
}

void cmd_0001(void)
{
	eeprom_update_if_changed(104, 0x01); // autoId OFF
}

void cmd_0100(void)
{
	eeprom_update_if_changed(105, '\0');
	gpib_set_readtermination('\0');
}

void cmd_0101(void)
{
	eeprom_update_if_changed(105, '\n');
	gpib_set_readtermination('\n');
}

void cmd_0102(void)
{
	eeprom_update_if_changed(105, '\r');
	gpib_set_readtermination('\r');
}

void cmd_addr(void)
{
}

void cmd_reset(void)
{
	Jump_To_Bootloader();
}

void cmd_string_short(void)
{
	eeprom_update_if_changed(106, 1);
}

void cmd_string_normal(void)
{
    eeprom_update_if_changed(106, 0);
}

void cmd_string_query(void)
{
	switch(eeprom_read_byte((uint8_t*)106))
	{
		case 0x01:
			set_internal_response((void*) "short", 5);
			break;
		default: // 0x00 or 0xff
			set_internal_response((void*)"normal", 6);
			break;
	}
}

PGM_P pparser_ps = cmd_parser;

void parser_reset(void)
{
    pparser_ps = cmd_parser;
}


bool parser_add(uint8_t ch)
{
	bool cmd_dispatched = false;
    uint8_t  entrycount;
	#if PARSER_ENTRYBYTECOUNT == 1
	uint8_t entryidx;
	#else
    uint16_t entryidx;
	#endif
    uint8_t cnt = 0;

	// enforce lower case
	if ( (ch >=65) && (ch<=90) )
		ch += 32;

	entrycount = pgm_read_byte(pparser_ps++);
    while (cnt < entrycount)
    {
        uint8_t entrych = pgm_read_byte(pparser_ps++);

		#if PARSER_ENTRYBYTECOUNT == 1
		entryidx = pgm_read_byte(pparser_ps++);
		#else
        entryidx = 0;
        for (uint8_t i=0; i < PARSER_ENTRYBYTECOUNT; i++)
        {
            entryidx <<= 8;
            entryidx = entryidx | pgm_read_byte(pparser_ps++);
        }
		#endif

        if ((entrych & 0x7F) == ch)
        {
            if ((entrych >= 128))
            { // entry to cmd list
				parser_func_t func;
				func = pgm_read_ptr_far(&(cmd_list[entryidx]));
				func(); // dispatch the command!
				cmd_dispatched = true;
            }
            else
            { // parsing not finished
                pparser_ps = &(cmd_parser[entryidx]);
				return false;
            }
        }

        cnt = cnt + 1;
    }

	// character not in list => restart parser at root
    parser_reset();
	return cmd_dispatched;
}
