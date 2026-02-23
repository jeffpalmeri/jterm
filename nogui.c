#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>

#include "ptyFork.h"
#include "from_book/lib/error_functions.h"

#define MAX_SNAME 1000
#define BUF_SIZE 256

int main(int argc, char *argv[]) {
  int masterFd;
  char slaveName[MAX_SNAME];
  pid_t childPid = ptyFork2(&masterFd, slaveName, MAX_SNAME);
  char *shell;

  fd_set inFds;
  ssize_t numRead;
  char buf[BUF_SIZE];

  if (childPid == -1) errExit("ptyFork");

  if (childPid == 0) {        /* Child: execute a shell on pty slave */
    printf("I'm the shell!!!\n");
      /* If the SHELL variable is set, use its value to determine
         the shell execed in child. Otherwise use /bin/sh. */

    shell = getenv("SHELL");
    if (shell == NULL || *shell == '\0')
        shell = "/bin/sh";

    execlp(shell, shell, (char *) NULL);
    errExit("execlp");      /* If we get here, something went wrong */
  }


  for (;;) {
      FD_ZERO(&inFds);
      FD_SET(STDIN_FILENO, &inFds);
      FD_SET(masterFd, &inFds);

      if (select(masterFd + 1, &inFds, NULL, NULL, NULL) == -1)
          errExit("select");

      if (FD_ISSET(STDIN_FILENO, &inFds)) {   /* stdin --> pty */
          numRead = read(STDIN_FILENO, buf, BUF_SIZE);
          if (numRead <= 0)
              exit(EXIT_SUCCESS);

          // fprintf(stderr, "\n[PTY OUTPUT %zd bytes]\n", numRead);
          // fwrite(buf, 1, numRead, stdout);

          // printf("*** numRead ?: %zi\n", numRead);
          // printf("This is the first read buf: %s\n", buf);

          if (write(masterFd, buf, numRead) != numRead)
              fatal("partial/failed write (masterFd)");
      }

      if (FD_ISSET(masterFd, &inFds)) {      /* pty --> stdout+file */
          numRead = read(masterFd, buf, BUF_SIZE);
          if (numRead <= 0)
              exit(EXIT_SUCCESS);

          // printf("Let me try to print the read buf: %s\n", buf);
          printf("How many came from the masterFd read: %zd\n", numRead);

          // for (ssize_t i = 0; i < numRead; i++) fprintf(stderr, "%02x ", (unsigned char)buf[i]);

          // fprintf(stderr, "\n");
          // dprintf(STDOUT_FILENO, "[PTY OUTPUT] %.*s", (int)numRead, buf);

          // if (write(STDOUT_FILENO, buf, numRead) != numRead)
          //     fatal("partial/failed write (STDOUT_FILENO)");

          // if (write(scriptFd, buf, numRead) != numRead)
          //     fatal("partial/failed write (scriptFd)");
      }
  }

  printf("made it to the end\n");
}
