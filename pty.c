#define _XOPEN_SOURCE 600
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include "pty.h"

int ptyMasterOpen(char *slaveName, size_t snLen)
{
  int masterFd, savedErrno;
  char *p;

  masterFd = posix_openpt(O_RDWR | O_NOCTTY); // Open pty master
  if(masterFd == -1) {
    return -1;
  }
  //
  if(grantpt(masterFd) == -1) { // Grant access to save pty
    savedErrno = errno;
    close(masterFd);
    errno = savedErrno;
    return -1;
  }

  if(unlockpt(masterFd) == -1) { // Unlock slave pty
    savedErrno = errno;
    close(masterFd);
    errno = savedErrno;
    return -1;
  }

  p = ptsname(masterFd); // Get slave pty name

  if(p == NULL) {
    savedErrno = errno;
    close(masterFd);
    errno = savedErrno;
    return -1;
  }

  if(strlen(p) < snLen) {
    strncpy(slaveName, p, snLen);
  } else { // Return an error if buffer is too small
    close(masterFd);
    errno = EOVERFLOW;
    return -1;
  }

  return masterFd;
}

