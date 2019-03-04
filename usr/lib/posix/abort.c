#include "stdlib.h"
#include "signal.h"
#include "unistd.h"

void abort(void) {
    raise(SIGABRT);
}
