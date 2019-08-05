#ifndef _IO_H
#define _IO_H 1

#include <sys/cdefs.h>

#ifdef __cplusplus
extern "C" {
#endif

static __inline unsigned char
inb(unsigned short int port)
{
  unsigned char _v;
  __asm__ __volatile__ ("inb %w1,%0":"=a" (_v):"Nd" (port));
  return _v;
}

static __inline void
outb(unsigned char value, unsigned short int port)
{
  __asm__ __volatile__ ("outb %b0,%w1": :"a" (value), "Nd" (port));
}

#ifdef __cplusplus
}
#endif

#endif
