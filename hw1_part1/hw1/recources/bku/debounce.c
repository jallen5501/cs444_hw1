/* debounce.c - a program to demonstrate software debouncing of a SPST switch */
/*  reads input from ACK pin and write output to D0 pin                       */
/*  use any key on console to terminate program, while switching back/forth   */

#include <stdio.h>
#include <cpu.h>   /* for inpt, outpt prototypes */
#include <lp.h>   

#define BUFLEN 80
void init_lp(void);
int detect_off(int msecs);
int detect_on(int msecs);
void delay(int msec);

void main()
{
  unsigned int m10thsecs;
  char buf[BUFLEN];
  int val = 0;			

  printf("Enter # of .1 msecs to delay: ");
  fgets(buf,BUFLEN,CONSOLE);	/* read in digits, newline */
  if (sscanf(buf,"%d",&m10thsecs)!=1) {
    printf("Need decimal number, exiting\n");
    return;
  }

  init_lp();			/* make sure LPT1 is in a known state */
 
  /* Loop until see char on console dev (when switching back/forth).
     Here the variable "val" tracks the official value, the software-
     debounced value of the switch */
  while (!readyc(CONSOLE)) {	/* ck console once a cycle */

    switch (val) {
    case 1:			/* now on, looking for off... */
      if ((val = detect_off(m10thsecs))==1)
	printf("Saw bounce back to 1!\n"); /* off, back on again */
      else
	printf("OFF ");		/* off and stayed off */
      break;
    case 0:			/* now off, looking for on... */
      if ((val = detect_on(m10thsecs))==0)
	printf("Saw bounce back to 0!\n"); /* on, back off again */
      else
	printf("ON ");		/* on and stayed on */
    }
  }
 
 /* Nothing needs to be undone for this simple device, you can't really
  * turn it off
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
  outpt(LPT1_BASE+LP_CNTRL, LP_PSELECP); /* set SLCTIN, other pins invert to 1*/
}

/* Clearly the following two routines are using the same basic logic,
 * but can you merge them into one readable function--a challenge! */

/* software-debounce on-to-off transition, apparently successfully if rets 0 */
int detect_off(int m10thsecs)
{
  /* loop until bit off */
  while  (inpt(LPT1_BASE+LP_STATUS) & LP_PACK) 
    ; 

  outpt(LPT1_BASE+LP_DATA, 0); /* write 0 bit back out */
  delay(m10thsecs);  /* after change seen, don't look for a while */
  if (inpt(LPT1_BASE+LP_STATUS) & LP_PACK) /* back on now? */
    {
      outpt(LPT1_BASE+LP_DATA, 1); /* echo new val */
      return 1;			/* new val, not consistent */
    }
  return 0;			/* consistent new val */
}

/* software-debounce off-to-on transition, apparently successfully if rets 1 */
int detect_on(int m10thsecs)
{
  /* loop until bit on */
  while ((inpt(LPT1_BASE+LP_STATUS) & LP_PACK)==0)
    ;
      
  outpt(LPT1_BASE+LP_DATA, 1); /* write 1 bit back out */
  delay(m10thsecs);  /* after change seen, don't look for a while */
  if ((inpt(LPT1_BASE+LP_STATUS) & LP_PACK)==0)
    {
      outpt (LPT1_BASE+LP_DATA, 0); /* echo new val */
      return 0;
    }
  return 1;			/* consistent new val */
}
	
/*************************************************************************
*
*  delay - without using timer, just CPU, approximate times, in .1 msec units.
*   Uses library call to make most of the execution time in the library code
*   so that it shouldn't make too much difference if *this* code is
*   is compiled with optimization flags such as -O or -O2.  Optimization
*   can affect times by a significant amount, maybe 50%.  (But note that
*   the library use disqualifies this as any kind of "benchmark".)
*/

#ifdef SAPC
/* 350 for 100Mhz 486, 3500 for 4M8z Pentium SAPC */
#define NLOOPS_PER_MSEC 3500
#else
/* for ulab (Sparc 10, would be greater on eris) */
#define NLOOPS_PER_MSEC 570
#endif

void delay( int m10thsecs ){
  char string1[] = "abcdefghij01234567890";
  char string2[sizeof(string1)];
  int i;

  for( i = (NLOOPS_PER_MSEC/10)*m10thsecs; i--; )
    strcpy(string2,string1);
  return;
}
