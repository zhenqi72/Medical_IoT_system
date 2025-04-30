#ifndef __DEFINES_H__
#define __DEFINES_H__

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#ifndef __cplusplus

#ifndef BOOL
#define BOOL            	bool
#endif

#ifndef TRUE
#define TRUE            	true
#endif

#ifndef FALSE
#define FALSE           	false
#endif

#endif

#ifndef NULL
#define NULL 				((void *)0)
#endif

#ifndef WEAK
#define WEAK				__attribute__((weak))
#endif

#ifndef PACKED
#define PACKED				__attribute__((__packed__))
#endif

#ifndef MIN
#define MIN(a,b)						((a) < (b) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a,b)						((a) > (b) ? (a) : (b))
#endif

#define NELEMENTS(x) 							((unsigned int)(sizeof(x)/sizeof(x[0])))
#define SIZEOF(s, m) 							((unsigned int)(sizeof(((s*)0)->m)))
#define ATTRIBUTE_OFFSET(structure,elem) 		((unsigned int)(&(((structure *)0)->elem)))
#define OFFSETOF(s, m) 							((unsigned int)(&(((s *)0)->m)))
#define CONTAINER(structure, elem, elem_addr)	((structure *)((((unsigned int)(elem_addr))) - ATTRIBUTE_OFFSET(structure, elem)))

#define U32_ALIGN4(a) 					((uint32_t)(((uint32_t)(a) + 3) & ~(uint32_t)(3)))
#define U32_ALIGN16(a) 					((uint32_t)(((uint32_t)(a) + 15) & ~(uint32_t)(15)))

#define in_range(c, lo, up)  					((uint8_t)(c) >= lo && (uint8_t)(c) <= up)
#define dec_char_value(c)						( (c)-'0' )
#define hex_char_value(c)						((uint8_t)(isdigit(c) ? dec_char_value(c) : (((c) >='A' && (c)<= 'F')? ((c)-'A' + 10) : ((c)-'a' + 10))))
int snprintf_hex(char *buf, size_t buf_size, const uint8_t *data, size_t len, char style);
#define snprintf_safe( buffer,size,format,... ) ({int r__ = snprintf(buffer,size,format,##__VA_ARGS__); (r__ > (size)) ? (size) : r__;})

#endif