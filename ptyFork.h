#include <sys/types.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <termios.h>

pid_t ptyFork(int *masterFd, char *slaveName, size_t snLen, const struct termios *slaveTermios, const struct winsize *slaveWS);

// Nothing term related
pid_t ptyFork2(int *masterFd, char *slaveName, size_t snLen);
