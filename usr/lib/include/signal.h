#pragma once
#include <uapi/signum.h>

typedef void (*sighandler_t)(int);

sighandler_t signal(int signum, sighandler_t handler);

void SIG_DFL(int signum);

void raise(int sig);
