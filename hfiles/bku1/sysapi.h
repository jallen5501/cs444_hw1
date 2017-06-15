/* sysapi.h: Device-independent i/o for Tutor and downloaded programs */
/* plus various device-specific calls as well. */
/* This file is included from stdio.h */
#ifndef SYSAPI_H
#define SYSAPI_H

/* for conditional compilation-- */
#define SAPC

#ifndef EOF
#define EOF (-1)
#endif

/* Used in getc and elsewhere--break out to Tutor.  By using a macro,
   we get a more useful report on the EIP when we get to Tutor */
#define breakpoint()  asm ( "int $3" )

/* for setting EOF from input from user */
#define EOFCHAR '\004'
/* for aborting user input, abort to Tutor--control-C */
#define ESCAPE_TO_TUTOR '\003'

/* Device numbers */
/* logical console */
#define CONSOLE 100
/* combination of monitor for output and keyboard for input */
#define KBMON 0
/* COM lines, i.e, RS232 serial lines */
#define COM1 1
#define COM2 2
#define COM3 3
#define COM4 4

/* Device-independent i/o API, using above device numbers */

/* init: init device driver, possibly claiming some memory starting
 * at mem_start, to whatever address is returned, but this is
 * done only the very first time init is called */
unsigned long init(int dev, unsigned long mem_start);

/* putc: output one char, with lf-> crlf, CONSOLE->console_dev, delay
 * if hostline, broadcase if dev < 0 */
int putc(int dev,char ch);

/* rawputc: output one char, by polling or equivalent, no interpretation
 * of char, but CONSOLE->console_dev mapping provided */
int rawputc(int dev, char ch);

/* getc: get one char from device by polling or equiv., convert CR
 * to '\n', abort if requested, echo if CONSOLE */
int getc(int dev);

/* rawgetc: get one char from device by polling or equiv., no interp. of
 * char, but CONSOLE->console_dev mapping provided */
int rawgetc(int dev);

/* readyc: check if char ready to be getc'd (returns Boolean),
 * with CONSOLE->console_dev mapping */
int readyc(int dev);

/* device independent i/o (polling) */
typedef unsigned long (*PFIL)(int, unsigned long);
typedef int (*PFI)(int);
typedef int (*PFIC)(int, char);
typedef struct dev_struct {
  PFIL devinit;
  PFIC devputc;			/* no interpretation */
  PFI devgetc;			/* no interpretation */
  PFI devreadyc;		/* return TRUE if char ready to read */
} Device;
/* leave room to grow */
#define MAXDEVS 10

/* The following struct contains the system API.  The kernel
   creates the master copy of this and the downloaded program
   obtains a pointer to it and uses it for system services */

typedef struct {
Device devtab[MAXDEVS];

/* in samain.c */
/* give the kernel a chance to reestablish essential handlers,
   called when returning from executing unknown code */
void (*enter_kernel)(void);

/* undo whatever had to be done by kernel, restoring handlers
   that the other code had established */
void (*exit_kernel)(void);

/* get device number of current Tutor console */
int (*get_console_dev)(void);

/* set Tutor console to a certain device */
void (*set_console_dev)(int dev);

/* get dev number of current Tutor hostline (line for putc slowdown) */
int (*get_hostline_dev)(void);

/* set Tutor hostline to certain device */
void (*set_hostline_dev)(int dev);

/* in saconsole.c */
/* return a string describing the monitor */
int (*cons_descript)(char *buf);

/* set vt100 emulation on monitor to 1 for on, 0 for off */
void (*cons_set_vt100_mode)(int mode);

/* get current vt100 emulation mode */
int (*cons_get_vt100_mode)(void);

/* in/for saserial.c */
/* read nbytes from comline, for fast service */
void (*com_readbuf)(int comline, char *buf, int nbytes);

/* get RS232 CTS line's current signal value */
int (*com_cts)(int comline);

/* get COM line's current baudrate from UART */
int (*com_get_baudrate)(int comline);

/* set COM line's baudrate */
int (*com_set_baudrate)(int comline, int baudrate);

/* get string describing UART */
int (*com_descript)(int dev, char *buf);

/* enable receiver interrupts and queue received characters for
   later pickup by getc.  Normally getc uses polling, no queuing */
void (*com_enable_rints)(int dev);

/* disable receiver interrupts, returning to normal getc processing */
void (*com_disable_rints)(int dev);

/* poll on both COM lines, shuttling chars back and forth, until
   escape char is received on console_line */
void (*com_fast_trans_mode)(int console_line, int host_line, char escape_char);

/* in sakeyboard.c */
/* reboot system */
void (*hard_reset_now)(void);
} SysAPI;
extern SysAPI *sysapi;		/* pointer to the interface */

#ifdef SAPC_KERNEL

/* sasys.h has these plus kernel-private inter-module prototypes */
#include <sasys.h>

#else

/* non-kernel code accesses these api functions via sysapi pointer-- */
/* sys_init is normally only used by kernel */
#define sys_init(dev, kmem_start) sysapi->devtab[dev].devinit(dev, kmem_start)
#define sys_getc(dev) sysapi->devtab[dev].devgetc(dev)
#define sys_putc(dev, ch) sysapi->devtab[dev].devputc(dev, ch)
#define sys_readyc(dev) sysapi->devtab[dev].devreadyc(dev)
#define sys_enter_kernel() sysapi->enter_kernel()
#define sys_exit_kernel() sysapi->exit_kernel()
#define sys_get_console_dev() sysapi->get_console_dev()
#define sys_set_console_dev(dev) sysapi->set_console_dev(dev)
#define sys_get_hostline_dev() sysapi->get_hostline_dev()
#define sys_set_hostline_dev(dev) sysapi->set_hostline_dev(dev)
#define sys_cons_descript(buf) sysapi->cons_descript(buf)
#define sys_cons_set_vt100_mode(mode) sysapi->cons_set_vt100_mode(mode)
#define sys_cons_get_vt100_mode() sysapi->cons_get_vt100_mode()
#define sys_com_readbuf(comline, buf, nbytes) \
                sysapi->com_readbuf(comline,buf,nbytes)
#define sys_com_cts(comline) sysapi->com_cts(comline)
#define sys_com_get_baudrate(comline) sysapi->com_get_baudrate(comline)
#define sys_com_set_baudrate(comline, baudrate) \
          sysapi->com_set_baudrate(comline, baudrate)
#define sys_com_descript(dev, buf) sysapi->com_descript(dev, buf)
#define sys_com_enable_rints(dev) sysapi->com_enable_rints(dev)
#define sys_com_disable_rints(dev) sysapi->com_disable_rints(dev)
#define sys_com_fast_trans_mode(console_line,host_line, escape_char) \
          sysapi->com_fast_trans_mode(console_line,host_line, escape_char)
#define sys_hard_reset_now() sysapi->hard_reset_now()
#endif /* !SAPC_KERNEL */
#endif /* SYSAPI_H */

