/* testlp.c: write to LPT1, read status, using outpt and inpt */
#include <stdio.h>
#include <cpu.h>   /* for inpt, outpt prototypes */
#include <lp.h>   
#define BUFLEN 80
void init_lp(void);
void report_lpstatus(void);

void main()
{
  unsigned int outchar = 1;
  char buf[BUFLEN];

  init_lp();			/* make sure it's in a known state */
  report_lpstatus();		/* look at its status bits */

  while (outchar) { 
    printf("Enter data byte in hex, 0 to end: "); 
    fgets(buf, BUFLEN, CONSOLE); 
    sscanf(buf, "%x", &outchar);
    printf("Doing OUT instruction to port %x...\n",LPT1_BASE+LP_DATA);
    outpt(LPT1_BASE+LP_DATA, outchar); /* write data byte to data reg in lp dev */
   } 

  printf("Done\n");
  /* nothing needs to be undone for this simple device, you can't really
   *   turn it off
   */
}

/* The device is always on, but make sure we know exactly what state it's in.
 * Actually, the device only interprets the DIR and interrupt-enable bits,
 * and just passes the other bits through to the output pins, inverting
 * all but SLCTIN on the way.
 */
void init_lp(void) 
{
  printf("Setting DIR bit off, no printer interrupts, output pins = 1\n");
  outpt(LPT1_BASE+LP_CNTRL, LP_PSELECP); /* set SLCTIN, other pins invert to 1 */
}

/* print to CONSOLE the lp status bits */
void report_lpstatus(void)
{
  unsigned char lpstatus;

  lpstatus = inpt(LPT1_BASE+LP_STATUS);	/* read status port for LPT1 */
  printf("lpstatus = %x\n",lpstatus);
  printf("LP_PBUSY bit = %d ",lpstatus&LP_PBUSY?1:0);
  /* this signal is inverted on the way through the device  */
  printf("means BUSY pin = %d\n",lpstatus&LP_PBUSY?0:1);
  printf("LP_PACK bit = ACK pin = %d\n",lpstatus&LP_PACK?1:0);
  printf("LP_POUTPA bit (out paper) = PE (paper end) pin = %d\n",
	 lpstatus&LP_POUTPA?1:0);
  printf("LP_PSELECD bit = SLCT pin = %d\n",lpstatus&LP_PSELECD?1:0);
  printf("LP_PERRORP bit = ERROR pin = %d\n",lpstatus&LP_PERRORP?1:0);
  printf("LP_PIRQSTAT bit (0 if int. active) = %d\n",lpstatus&LP_PIRQSTAT?1:0);
}
