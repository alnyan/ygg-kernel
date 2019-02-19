#pragma once
#include "sys/mm.h"

int elf_load(mm_pagedir_t dst, uintptr_t src_addr, size_t src_len);

