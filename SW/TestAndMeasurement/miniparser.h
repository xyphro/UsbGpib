#ifndef MINIPARSER_H_
#define MINIPARSER_H_

#include <stdint.h>
#include <stdbool.h>
 
/* reset parser */
void parser_reset(void);

/* parse a character. When a command is fully received, the command is also executed*/
/* return: true when a command was dispatched, otherwise false */
bool parser_add(uint8_t ch);

/* called to finalize parser. Required to allow parsing for numeric payload for addr command */
void parser_finalize(void);
 
#endif /* MINIPARSER_H_ */
