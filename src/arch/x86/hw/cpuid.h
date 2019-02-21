#pragma once
#include <stdint.h>

#define CPUID_FEAT_SSE3         0
#define CPUID_FEAT_PCLMUL       1
#define CPUID_FEAT_DTES64       2
#define CPUID_FEAT_MONITOR      3
#define CPUID_FEAT_DS_CPL       4
#define CPUID_FEAT_VMX          5
#define CPUID_FEAT_SMX          6
#define CPUID_FEAT_EST          7
#define CPUID_FEAT_TM2          8
#define CPUID_FEAT_SSSE3        9
#define CPUID_FEAT_CID          10
#define CPUID_FEAT_FMA          12
#define CPUID_FEAT_CX16         13
#define CPUID_FEAT_ETPRD        14
#define CPUID_FEAT_PDCM         15
#define CPUID_FEAT_PCIDE        17
#define CPUID_FEAT_DCA          18
#define CPUID_FEAT_SSE4_1       19
#define CPUID_FEAT_SSE4_2       20
#define CPUID_FEAT_x2APIC       21
#define CPUID_FEAT_MOVBE        22
#define CPUID_FEAT_POPCNT       23
#define CPUID_FEAT_AES          25
#define CPUID_FEAT_XSAVE        26
#define CPUID_FEAT_OSXSAVE      27
#define CPUID_FEAT_AVX          28

#define CPUID_FEAT_FPU          32
#define CPUID_FEAT_VME          33
#define CPUID_FEAT_DE           34
#define CPUID_FEAT_PSE          35
#define CPUID_FEAT_TSC          36
#define CPUID_FEAT_MSR          37
#define CPUID_FEAT_PAE          38
#define CPUID_FEAT_MCE          39
#define CPUID_FEAT_CX8          40
#define CPUID_FEAT_APIC         41
#define CPUID_FEAT_SEP          43
#define CPUID_FEAT_MTRR         44
#define CPUID_FEAT_PGE          45
#define CPUID_FEAT_MCA          46
#define CPUID_FEAT_CMOV         47
#define CPUID_FEAT_PAT          48
#define CPUID_FEAT_PSE36        49
#define CPUID_FEAT_PSN          50
#define CPUID_FEAT_CLF          51
#define CPUID_FEAT_DTES         53
#define CPUID_FEAT_ACPI         54
#define CPUID_FEAT_MMX          55
#define CPUID_FEAT_FXSR         56
#define CPUID_FEAT_SSE          57
#define CPUID_FEAT_SSE2         58
#define CPUID_FEAT_SS           59
#define CPUID_FEAT_HTT          60
#define CPUID_FEAT_TM1          61
#define CPUID_FEAT_IA64         62
#define CPUID_FEAT_PBE          63

int x86_cpuid_check_feature(uint32_t num);
void x86_cpuid_dump_features(void);

void x86_cpuid_init(void);
