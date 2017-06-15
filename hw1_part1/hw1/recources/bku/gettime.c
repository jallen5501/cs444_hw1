
/* gettime.c  Get time from RTC, S&S, p. 399 */
/* build with $pcex/makefile:  make C=gettime */

#include <stdio.h>
/* NOTE: This code needs cleaning up by replacing wired-in numbers by
   properly defined constants. */
/* #include <rtclock.h> */

void delay(void);
unsigned char readp(unsigned char reg);
void writep(unsigned char reg, unsigned char val);
void gettime(unsigned char* hrs, unsigned char *mins, unsigned char *secs);

void main()
{
  unsigned char hr, min, sec;

  gettime(&hr, &min, &sec);
  /* numbers are in BCD, so 4 bits/digit, like hex-- */
  printf("%02x:%02x:%02x\n",hr, min, sec); /* print leading 0's, 2 cols */
}

/* about 10 usecs */
void delay(void)
{
  int i;
  
  for (i=0;i<1000;i++)
    ;
}

/* read reg from RTC */
unsigned char readp(unsigned char reg)
{
  outpt(0x70, reg);
  return inpt(0x71);
}

/* write reg in RTC */
/* Warning: this writes in non-volatile memory and can disable the PC,
   requiring running of BIOS "setup" program to fix it. */
void writep(unsigned char reg, unsigned char val)
{
  outpt(0x70, reg);
  outpt(0x71, val);
}

/* read time from RTC */
void gettime(unsigned char* hrs, unsigned char *mins, unsigned char *secs)
{
  while (readp(0xa)&0x80)
    delay();			/* update in progress, wait */
  *secs = readp(0);
  *mins = readp(2);
  *hrs = readp(4);
}
