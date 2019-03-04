#include "signal.h"
#include "unistd.h"
#include "stdio.h"
#include <uapi/syscall.h>

static sighandler_t psignals[16] = {};

void SIG_IGN(int signum) {
    printf("Ignored a signal\n");
}

void SIG_DFL(int signum) {
    // Print basic info about what happened
    switch (signum) {
    case SIGKILL:
        printf("Killed\n");
        break;
    case SIGABRT:
        printf("abort() was called\n");
        break;
    case SIGSEGV:
        printf("Segmentation fault\n");
        break;
    default:
        printf("Unhandled signal: %d\n", signum);
        break;
    }
    exit(1);
}

static void __libc_signal_handle(void) {
    // The kernel doesn't use the stack, so it sets %edx to signum in sigctx
    int signum;
    asm volatile ("":"=d"(signum));

    if (signum >= sizeof(psignals) / sizeof(psignals[0])) {
        SIG_DFL(signum);
    } else {
        psignals[signum](signum);
    }

    asm volatile ("int $0x80"::"a"(SYSCALL_NRX_SIGRETURN));
}

void __libc_signal_init(void) {
    for (int i = 0; i < sizeof(psignals) / sizeof(psignals[0]); ++i) {
        psignals[i] = SIG_DFL;
    }

    // Set signal handler in kernel
    asm volatile ("int $0x80"::"a"(SYSCALL_NRX_SIGNAL),"b"(0),"c"(__libc_signal_handle));
}

sighandler_t signal(int signum, sighandler_t newh) {
    if (signum >= sizeof(psignals) / sizeof(psignals[0])) {
        return SIG_DFL;
    }
    sighandler_t old = psignals[signum];
    psignals[signum] = newh;
    return old;
}

void raise(int sig) {
    // TODO: move this to kill(getpid(), ...)
    asm volatile ("int $0x80"::"a"(SYSCALL_NRX_RAISE),"b"(sig));
}
