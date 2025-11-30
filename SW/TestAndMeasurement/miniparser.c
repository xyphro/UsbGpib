
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
static const char cmd_parser[] PROGMEM = {0x06, 'a' , 0x0d, 't' , 0x4c, 'v' , 0x7b, '0' , 
    0x84, 'r' , 0x9b, 's' , 0xa7, 0x02, 'u' , 0x12, 'd' , 0x46, 0x01, 't' , 0x15, 0x01, 
    'o' , 0x18, 0x01, 'i' , 0x1b, 0x01, 'd' , 0x1e, 0x02, ' ' , 0x23, '?'|0x80 , 0x05, 
    0x02, 'o' , 0x28, 's' , 0x30, 0x02, 'n'|0x80 , 0x00, 'f' , 0x2d, 0x01, 'f'|0x80 , 
    0x01, 0x01, 'l' , 0x33, 0x01, 'o' , 0x36, 0x01, 'w' , 0x39, 0x02, '_'|0x80 , 0x02, 
    'e' , 0x3e, 0x02, 'r'|0x80 , 0x03, 's' , 0x43, 0x01, 't'|0x80 , 0x04, 0x01, 'd' , 
    0x49, 0x01, 'r'|0x80 , 0x11, 0x01, 'e' , 0x4f, 0x01, 'r' , 0x52, 0x01, 'm' , 0x55, 
    0x02, ' ' , 0x5a, '?'|0x80 , 0x09, 0x04, 'c' , 0x63, 'l' , 0x66, 'e' , 0x69, 's' , 
    0x6f, 0x01, 'r'|0x80 , 0x06, 0x01, 'f'|0x80 , 0x07, 0x01, 'o' , 0x6c, 0x01, 'i'|0x80 , 
    0x08, 0x01, 't' , 0x72, 0x01, 'o' , 0x75, 0x01, 'r' , 0x78, 0x01, 'e'|0x80 , 0x0a, 
    0x01, 'e' , 0x7e, 0x01, 'r' , 0x81, 0x01, '?'|0x80 , 0x0b, 0x02, '0' , 0x89, '1' , 
    0x91, 0x01, '0' , 0x8c, 0x02, '0'|0x80 , 0x0c, '1'|0x80 , 0x0d, 0x01, '0' , 0x94, 
    0x03, '0'|0x80 , 0x0e, '1'|0x80 , 0x0f, '2'|0x80 , 0x10, 0x01, 'e' , 0x9e, 0x01, 
    's' , 0xa1, 0x01, 'e' , 0xa4, 0x01, 't'|0x80 , 0x12, 0x01, 't' , 0xaa, 0x01, 'r' , 
    0xad, 0x01, 'i' , 0xb0, 0x01, 'n' , 0xb3, 0x01, 'g' , 0xb6, 0x02, ' ' , 0xbb, '?'|0x80 , 
    0x15, 0x02, 's' , 0xc0, 'n' , 0xcc, 0x01, 'h' , 0xc3, 0x01, 'o' , 0xc6, 0x01, 'r' , 
    0xc9, 0x01, 't'|0x80 , 0x13, 0x01, 'o' , 0xcf, 0x01, 'r' , 0xd2, 0x01, 'm' , 0xd5, 
    0x01, 'a' , 0xd8, 0x01, 'l'|0x80 , 0x14 };

void cmd_autoid_on(void);
void cmd_autoid_off(void);
void cmd_autoid_slow_(void);
void cmd_autoid_slower(void);
void cmd_autoid_slowest(void);
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
    cmd_autoid_slow_, 
    cmd_autoid_slower, 
    cmd_autoid_slowest, 
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

void cmd_autoid_slow_(void)
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
			set_internal_response((void*)"slowest", 7);
			break;
		case 0x03:
			set_internal_response((void*)"slower", 6);
			break;
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
	set_internal_response((void*)"V2.2", 4);
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
