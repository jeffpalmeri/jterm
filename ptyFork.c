#include "pty.h"
#include "ptyFork.h"

#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <termios.h>

#define MAX_SNAME 1000

pid_t ptyFork(int *masterFd, char *slaveName, size_t snLen, const struct termios *slaveTermios, const struct winsize *slaveWS)
{
  int mfd, slaveFd, savedErrno;
  pid_t childPid;
  char slname[MAX_SNAME];

  mfd = ptyMasterOpen(slname, MAX_SNAME);
  if(mfd == -1) return -1;

  if(slaveName != NULL) { // Return slave name to caller
    if(strlen(slname) < snLen) {
      strncpy(slaveName, slname, snLen);
    } else { // Slave name was too small
      close(mfd);
      errno = EOVERFLOW;
      return -1;
    }
  }

  childPid = fork();

  if(childPid == -1) { // fork() failed
    savedErrno = errno;
    close(mfd);
    errno = savedErrno;
    return -1;
  }

  if(childPid != 0) { // Parent
    *masterFd = mfd; // Only parent gets fd
    return childPid;
  }

  // Child falls through to here
  if(setsid() == -1) { // Start a new session
    return -5;
  }

  close(mfd); // Not needed in child

  slaveFd = open(slname, O_RDWR); // Becomes controlling tty
  if(slaveFd == -1) {
    return -6;
  }

  #ifdef TIOCSCTTY
    if(ioctl(slaveFd, TIOCSCTTY, 0) == -1) return -7;
  #endif

  if(slaveTermios != NULL) {
    if(tcsetattr(slaveFd, TCSANOW, slaveTermios) == -1) { // Set slave tty attributes
      return -7;
    }
  }

  if(slaveWS != NULL) {
    if(ioctl(slaveFd, TIOCSWINSZ, slaveWS) == -1) { // Set slave tty window
      return -8;
    }
  }

  // Duplicate pty slave to be child's stdin, stdout, and stderr
  if(dup2(slaveFd, STDIN_FILENO) != STDIN_FILENO) return -9;
  if(dup2(slaveFd, STDOUT_FILENO) != STDOUT_FILENO) return -10;
  if(dup2(slaveFd, STDERR_FILENO) != STDERR_FILENO) return -11;

  if(slaveFd > STDERR_FILENO) close(slaveFd); // Safety check, slaveFd no longer needed

  return 0; // Like child of fork()
}


pid_t ptyFork2(int *masterFd, char *slaveName, size_t snLen)
{
  int mfd, slaveFd, savedErrno;
  pid_t childPid;
  char slname[MAX_SNAME];

  mfd = ptyMasterOpen(slname, MAX_SNAME);
  if(mfd == -1) return -1;

  if(slaveName != NULL) { // Return slave name to caller
    if(strlen(slname) < snLen) {
      strncpy(slaveName, slname, snLen);
    } else { // Slave name was too small
      close(mfd);
      errno = EOVERFLOW;
      return -1;
    }
  }

  childPid = fork();

  if(childPid == -1) { // fork() failed
    savedErrno = errno;
    close(mfd);
    errno = savedErrno;
    return -1;
  }

  if(childPid != 0) { // Parent
    *masterFd = mfd; // Only parent gets fd
    return childPid;
  }

  // Child falls through to here
  if(setsid() == -1) { // Start a new session
    return -5;
  }

  close(mfd); // Not needed in child

  slaveFd = open(slname, O_RDWR); // Becomes controlling tty
  if(slaveFd == -1) {
    return -6;
  }

  #ifdef TIOCSCTTY
    if(ioctl(slaveFd, TIOCSCTTY, 0) == -1) return -7;
  #endif

  // Duplicate pty slave to be child's stdin, stdout, and stderr
  if(dup2(slaveFd, STDIN_FILENO) != STDIN_FILENO) return -9;
  if(dup2(slaveFd, STDOUT_FILENO) != STDOUT_FILENO) return -10;
  if(dup2(slaveFd, STDERR_FILENO) != STDERR_FILENO) return -11;

  if(slaveFd > STDERR_FILENO) close(slaveFd); // Safety check, slaveFd no longer needed

  return 0; // Like child of fork()
}
