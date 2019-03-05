#include "fpu.h"
#include "sys/debug.h"
#include "cpuid.h"

extern void x86_sse_enable(void);

int fpu_init(void) {
    if (x86_cpuid_check_feature(CPUID_FEAT_SSE)) {
        kdebug("Enabling SSE support\n");
        x86_sse_enable();

        float f1[4] = { 1.0f, 2.0f, 3.0f, 4.0f };
        float f2[4] = { 1.0f, 2.0f, 3.0f, 4.0f };
        float f3[4];

        asm volatile (" \
                movaps (%0), %%xmm0; \
                movaps (%1), %%xmm1; \
                addps %%xmm1, %%xmm0; \
                movaps %%xmm0, (%2); \
                "::
                "a"(f1),
                "b"(f2),
                "c"(f3));

        for (int i = 0; i < 4; ++i) {
            kdebug("f1[%d] = %d\n", i, (int) f1[i]);
        }
        for (int i = 0; i < 4; ++i) {
            kdebug("f2[%d] = %d\n", i, (int) f2[i]);
        }
        for (int i = 0; i < 4; ++i) {
            kdebug("f3[%d] = %d\n", i, (int) f3[i]);
        }
    }

    return 0;
}
