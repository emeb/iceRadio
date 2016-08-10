/*
 * libstubs.c - stubs that the library wants since libnosys doesn't do anything
 */

#include <stdio.h>
#include <errno.h>
#include "usart.h"

/* _end is set in the linker command file
extern caddr_t _end;*/

/* just in case, most boards have at least some memory */
#ifndef RAMSIZE
#  define RAMSIZE             (caddr_t)0x100000
#endif

/*
 * sbrk -- changes heap size size. Get nbytes more
 *         RAM. We just increment a pointer in what's
 *         left of memory on the board.
 */
caddr_t
_sbrk(nbytes)
     int nbytes;
{
  static caddr_t heap_ptr = NULL;
  caddr_t        base;

  if (heap_ptr == NULL) {
    heap_ptr = (caddr_t)0x20007fff;
  }

  if ((RAMSIZE - heap_ptr) >= 0) {
    base = heap_ptr;
    heap_ptr += nbytes;
    return (base);
  } else {
    errno = ENOMEM;
    return ((caddr_t)-1);
  }
}

/*
 * isatty -- returns 1 if connected to a terminal device,
 *           returns 0 if not. Since we're hooked up to a
 *           serial port, we'll say yes and return a 1.
 */
int
_isatty(fd)
     int fd;
{
  return (1);
}

/*
 * getpid -- only one process, so just return 1.
 */
#define __MYPID 1
int
_getpid()
{
  return __MYPID;
}

/*
 * exit
 */
void
_exit(int val)
{
	while(1);
}

/*
 * kill -- go out via exit...
 */
int
_kill(pid, sig)
     int pid;
     int sig;
{
  if(pid == __MYPID)
    _exit(sig);
  return 0;
}

/*
 * read  -- read bytes from the serial port. Ignore fd, since
 *          we only have stdin.
 */
int
_read(fd, buf, nbytes)
     int fd;
     char *buf;
     int nbytes;
{
  int i = 0;

  for (i = 0; i < nbytes; i++) {
    *(buf + i) = inbyte();
    if ((*(buf + i) == '\n') || (*(buf + i) == '\r')) {
      (*(buf + i)) = 0;
      break;
    }
  }
  return (i);
}

/*
 * write -- write bytes to the serial port. Ignore fd, since
 *          stdout and stderr are the same. Since we have no filesystem,
 *          open will only return an error.
 */
int
_write(fd, buf, nbytes)
     int fd;
     char *buf;
     int nbytes;
{
  int i;

  for (i = 0; i < nbytes; i++) {
    if (*(buf + i) == '\n') {
      outbyte ('\r');
    }
    outbyte (*(buf + i));
  }
  return (nbytes);
}


/*
 * close -- close a file descriptor. We don't need
 *          to do anything, but pretend we did.
 */
int
_close(fd)
     int fd;
{
  return (0);
}

/*
 * lseek -- move read/write pointer. Since a serial port
 *          is non-seekable, we return an error.
 */
off_t
_lseek(fd,  offset, whence)
     int fd;
     off_t offset;
     int whence;
{
  errno = ESPIPE;
  return ((off_t)-1);
}

/*
 * fstat -- get status of a file. Since we have no file
 *          system, we just return an error.
 */
int
_fstat(fd, buf)
     int fd;
     struct stat *buf;
{
  errno = EIO;
  return (-1);
}
