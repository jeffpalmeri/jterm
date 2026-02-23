#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <libgen.h>
#include <termios.h>
#include <sys/select.h>
#include "ptyFork.h"
#include "tty_functions.h"

#define BUF_SIZE 256
#define MAX_SNAME 1000

struct termios ttyOrig;

void script_die(const char *errString, ...) {
  va_list argPtr;

  va_start(argPtr, errString);
  vfprintf(stderr, errString, argPtr);
  va_end(argPtr);

  exit(1);
}

/*
 * Reset terminal mode on exit
 */
static void
ttyReset(void)
{
  if(tcsetattr(STDIN_FILENO, TCSANOW, &ttyOrig) == -1) {
    script_die("tcsetattr");
  }
}

int main(int argc, char *argv[])
{
  char slaveName[MAX_SNAME];
  char *shell;
  int masterFd, scriptFd;
  struct winsize ws;
  fd_set inFds;
  char buf[BUF_SIZE];
  ssize_t numRead;
  pid_t childPid;

  if(tcgetattr(STDIN_FILENO, &ttyOrig) == -1) {
    script_die("tcgetattr");
  }

  // TIOCGWINSZ = Terminal IO Control Get Window Size
  if(ioctl(STDIN_FILENO, TIOCGWINSZ, &ws) < 0) {
    script_die("ioctl-TIOCGWINSZ");
  }

  childPid = ptyFork(&masterFd, slaveName, MAX_SNAME, &ttyOrig, &ws);
  if(childPid == -1) {
    script_die("ptyFork");
  }

  if(childPid == 0) {
    shell = getenv("SHELL");
    if(shell == NULL || *shell == '\0') shell = "/bin/sh";

    execlp(shell, shell, (char *) NULL);
    script_die("execlp"); // If we get here, something went wrong
  }

  scriptFd = open((argc > 1) ? argv[1] : "typescript",
                     O_WRONLY | O_CREAT | O_TRUNC,
                     S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);

  if(scriptFd == -1) {
    script_die("script_die: open typescript\n");
  }

  ttySetRaw(STDIN_FILENO, &ttyOrig);
  
  if(atexit(ttyReset) != 0) {
    script_die("atexit");
  }

  for(;;) {
    FD_ZERO(&inFds);
    FD_SET(STDIN_FILENO, &inFds);
    FD_SET(masterFd, &inFds);

    if(select(masterFd + 1, &inFds, NULL, NULL, NULL) == -1) {
      script_die("select");
    }

    // I am DUMB
    // if(FD_ISSET(STDIN_FILENO, &inFds)) { // stdin --> pty
    //   numRead = read(STDIN_FILENO, buf, BUF_SIZE);
    //   if(numRead <= 0) exit(EXIT_SUCCESS);
    //
    //   if(write(masterFd, buf, numRead) != numRead) {
    //     script_die("partial/failed write (masterFd)");
    //   }
    //   // pty --> stdout+file
    //   if(FD_ISSET(masterFd, &inFds)) {
    //     numRead = read(masterFd, buf, BUF_SIZE);
    //     if(numRead <= 0) exit(EXIT_SUCCESS);
    //
    //     if(write(STDOUT_FILENO, buf, numRead) != numRead) {
    //       script_die("partial/failed write (STDOUT_FILENO)");
    //     }
    //
    //     if(write(scriptFd, buf, numRead) != numRead) {
    //       script_die("partial/failed write (scriptFd)");
    //     }
    //   }
    // }
    if(FD_ISSET(STDIN_FILENO, &inFds)) { // stdin --> pty
      numRead = read(STDIN_FILENO, buf, BUF_SIZE);
      if(numRead <= 0) exit(EXIT_SUCCESS);

      if(write(masterFd, buf, numRead) != numRead) {
        script_die("partial/failed write (masterFd)");
      }
      // pty --> stdout+file
    }
    if(FD_ISSET(masterFd, &inFds)) {
      numRead = read(masterFd, buf, BUF_SIZE);
      if(numRead <= 0) exit(EXIT_SUCCESS);

      if(write(STDOUT_FILENO, buf, numRead) != numRead) {
        script_die("partial/failed write (STDOUT_FILENO)");
      }

      if(write(scriptFd, buf, numRead) != numRead) {
        script_die("partial/failed write (scriptFd)");
      }
    }
  }
}



