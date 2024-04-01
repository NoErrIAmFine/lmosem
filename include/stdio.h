#ifndef __STDIO_H
#define __STDIO_H

#ifndef _VALIST
#define _VALIST
typedef char *va_list;
#endif /* _VALIST */

#include <types/basetype.h>

int printf(const char *fmt, ...);
int scanf(const char * fmt, ...);

extern int vsnprintf(char *buf, size_t size, const char *fmt, va_list args);
extern int snprintf(char * buf, size_t size, const char *fmt, ...);
extern int vsprintf(char *buf, const char *fmt, va_list args);
extern int sprintf(char * buf, const char *fmt, ...);
extern int vsscanf(const char * buf, const char * fmt, va_list args);
extern int sscanf(const char * buf, const char * fmt, ...);

extern void putc(unsigned char c);
extern unsigned char getc(void);

#endif // __STDIO_H