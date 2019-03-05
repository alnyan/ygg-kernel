#include "signal.h"
#include "unistd.h"
#include "stdio.h"
#include <uapi/syscall.h>

static sighandler_t psignals[16] = {};
static sighandler_t usignals[2] = {};

void SIG_IGN(int signum) {
    printf("Ignored a signal\n");
}

void SIG_DFL(int signum) {
    // Print basic info about what happened
    switch (signum) {
    case SIGKILL:
        printf("%d: killed\n", getpid());
        break;
    case SIGABRT:
        printf("%d: abort() was called\n", getpid());
        break;
    case SIGSEGV:
        printf("%d: segmentation fault\n", getpid());
        break;
    default:
        printf("%d: unhandled signal: %d\n", getpid(), signum);
        break;
    }
    exit(1);
}

static void __libc_signal_handle(void) {
    // The kernel doesn't use the stack, so it sets %edx to signum in sigctx
    int signum;
    asm volatile ("":"=d"(signum));

    if (signum == SIGUSR1 || signum == SIGUSR2) {
        usignals[signum - SIGUSR1](signum);
    } else if (signum >= sizeof(psignals) / sizeof(psignals[0])) {
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
    usignals[0] = SIG_DFL;
    usignals[1] = SIG_DFL;

    // Set signal handler in kernel
    asm volatile ("int $0x80"::"a"(SYSCALL_NRX_SIGNAL),"b"(0),"c"(__libc_signal_handle));
}

sighandler_t signal(int signum, sighandler_t newh) {
    sighandler_t old;

    if (signum == SIGUSR1 || signum == SIGUSR2) {
        old = usignals[signum - SIGUSR1];
        usignals[signum - SIGUSR1] = newh;
    } else if (signum >= sizeof(psignals) / sizeof(psignals[0])) {
        return SIG_DFL;
    } else {
        old = psignals[signum];
        psignals[signum] = newh;
    }
    return old;
}

void raise(int sig) {
    // TODO: move this to kill(getpid(), ...)
    asm volatile ("int $0x80"::"a"(SYSCALL_NRX_RAISE),"b"(sig));
}
