/* printf, fgets, dprintf*/
#include <stdio.h>
/* system() */
#include <stdlib.h>
/*uint8_t etc. */
#include <stdint.h>

/* printf format string for expression.c */
#include "shellcode.h"


#define expression_buflen 1024
#define shellcode_buflen  2048



void callback_2(uint8_t val);
void call_this(void);
char shellcode_buf[shellcode_buflen];
