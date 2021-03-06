
/* echo.c: use low-level i/o to echo console input */
/* build with makefile in $pcex:  make C=echo      */

#include <stdio.h>
#include <serial.h>

void main()
{
  int console = sys_get_console_dev(); /* find out current sys console */
  int conport;
  int c = 0;

  switch (console) {
  case COM1: conport = COM1_BASE;
    break;
  case COM2: conport = COM2_BASE;
    break;
  default: printf("Expected serial port for console, exiting\n");
    return;
  }

  while (c != 'q') {

    /* "poll" the DR (data-ready) bit until it goes on */
    while ((inpt(conport+UART_LSR)&UART_LSR_DR)==0)
      ;				/* busy-wait for char */
    c = inpt(conport+UART_RX);
    outpt(conport+UART_TX, c);	/* should be ready */
  }
}
