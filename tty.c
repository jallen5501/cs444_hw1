/* transmit interrupt: when sending data. turn the interrupt on, when now tranmitting data, turn the interrupt off  */
/* when you read the UART_RX register, the bits of the register are reset to 0 */
/*********************************************************************
*       file:           tty.c
*       author:         betty o'neil
*
*       tty driver--device-specific routines for ttys */
#include <stdio.h>  /* for kprintf prototype */
#include <serial.h>
#include <cpu.h>
#include <pic.h>
#include "ioconf.h"
#include "tty_public.h"
#include "tty.h"
#include "queue/queue.h"

struct tty ttytab[NTTYS];        /* software params/data for each SLU dev */
/* tell C about the assembler shell routines */
extern void irq3inthand(void), irq4inthand(void);
/* C part of interrupt handlers--specific names called by the assembler code */
extern void irq3inthandc(void), irq4inthandc(void); 
/* the common code for the two interrupt handlers */         
static void irqinthandc(int dev); 
/*===================================================================*/
Queue qin, qout, echoq;
/*       tty specific initialization routine for COM devices         */
void ttyinit(int dev) {

  int baseport;
  struct tty *tty;		/* ptr to tty software params/data block */
  init_queue(&qin, MAXBUF); 
  init_queue(&qout, MAXBUF); 
  init_queue(&echoq, MAXBUF);

  baseport = devtab[dev].dvbaseport; /* pick up hardware addr */
  tty = (struct tty *)devtab[dev].dvdata; /* and software params struct */

  if (baseport == COM1_BASE) {
      /* arm interrupts by installing int vec */
     set_intr_gate(COM1_IRQ+IRQ_TO_INT_N_SHIFT, &irq4inthand);
      pic_enable_irq(COM1_IRQ);
  } else if (baseport == COM2_BASE) {
      /* arm interrupts by installing int vec */
      set_intr_gate(COM2_IRQ+IRQ_TO_INT_N_SHIFT, &irq3inthand);
      pic_enable_irq(COM2_IRQ);
  } else {
      kprintf("Bad TTY device table entry, dev %d\n", dev);
      return;			/* give up */
  }
  tty->echoflag = 1;		/* default to echoing */
  /* enable interrupts on receiver */
  outpt(baseport+UART_IER, UART_IER_RDI); /* RDI = receiver data int */
}

/*       tty-specific read routine for TTY devices                   */
int ttyread(int dev, char *buf, int nchar) {
  int baseport, ch, i;
  struct tty *tty;
  int saved_eflags;        /* old cpu control/status reg, so can restore it */

  baseport = devtab[dev].dvbaseport; /* hardware addr from devtab */
  tty = (struct tty *)devtab[dev].dvdata;   /* software data for line */
  saved_eflags = get_eflags();

  i = 0;
  while (i < nchar) {
    cli();
    if ((ch = dequeue(&qin)) != EMPTYQUE) {
      buf[i] = ch;
      i++;
    }
    set_eflags(saved_eflags);
  }
  return nchar;       /* but should wait for rest of nchar chars if nec. */
}
/*====================================================================
*       tty-specific write routine for SAPC devices                 */

int ttywrite(int dev, char *buf, int nchar) {
  int baseport, i;
  struct tty *tty;
  baseport = devtab[dev].dvbaseport; /* hardware addr from devtab */
  tty = (struct tty *)devtab[dev].dvdata;   /* software data for line */

  cli();
  i = 0;
  while (i < nchar) {
    if (enqueue(&qout, buf[i]) != FULLQUE) {
      outpt(baseport+UART_IER, UART_IER_RDI | UART_IER_THRI);
      i++;
    }
  sti(); 
  }
  return nchar;
}

/*       tty-specific control routine for TTY devices                */
int ttycontrol(int dev, int fncode, int val) {
  struct tty *this_tty = (struct tty *)(devtab[dev].dvdata);

  if (fncode == ECHOCONTROL)
    this_tty->echoflag = val;
  else return -1;
  return 0;
}

/*  tty-specific interrupt routine for COM ports                      */
void irq4inthandc() {	irqinthandc(TTY0);	}                           
void irq3inthandc() {	irqinthandc(TTY1);	}                          

void irqinthandc(int dev){  
  int baseport, ch;
  byte lsr;
  struct tty *tty = (struct tty *)(devtab[dev].dvdata);
  baseport = devtab[dev].dvbaseport; /* hardware i/o port */;

  pic_end_int();  	/*notify pic that its part is done */ 
  lsr = inpt(baseport+UART_LSR); 
  if (lsr & UART_LSR_DR) {		/* handling recieve interrupt */
    ch = inpt(baseport+UART_RX);	/* read char, ask the device */
    enqueue(&qin, ch);
    if (tty->echoflag)	enqueue(&echoq, ch);
  } 
  if (lsr & UART_LSR_THRE) {	/* handling transmit interrupt */
    if (queuecount(&echoq))	outpt(baseport+UART_TX, dequeue(&echoq));
    if (queuecount(&qout)) {
     ch = dequeue(&qout);
     outpt(baseport + UART_TX, ch);
    }  
  }
  outpt(baseport+UART_IER, UART_IER_RDI);
}
