// This is free and unencumbered software released into the public domain.
//
// Anyone is free to copy, modify, publish, use, compile, sell, or
// distribute this software, either in source code form or as a compiled
// binary, for any purpose, commercial or non-commercial, and by any
// means.

#include <stdarg.h>
#include <stdint.h>

extern long time();
// extern long insn();

extern char *malloc();
extern int printf(const char *format, ...);

char heap_memory[1024];
int heap_memory_used = 0;

long time()
{
	int cycles;
	asm volatile ("rdcycle %0" : "=r"(cycles));
	// printf("[time() -> %d]", cycles);
	return cycles;
}

// long insn()
// {
// 	int insns;
// 	asm volatile ("rdinstret %0" : "=r"(insns));
// 	// printf("[insn() -> %d]", insns);
// 	return insns;
// }

char *malloc(int size)
{
	char *p = heap_memory + heap_memory_used;
	// printf("[malloc(%d) -> %d (%d..%d)]", size, (int)p, heap_memory_used, heap_memory_used + size);
	heap_memory_used += size;
	if (heap_memory_used > 1024)
		asm volatile ("ebreak");
	return p;
}
