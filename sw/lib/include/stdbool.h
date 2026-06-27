#pragma once

/** Expands to the integer constant 1. */
#define true 1

/** Expands to the integer constant 0. */
#define false 0

/** Expands to _Bool. */
#if !defined(__cplusplus) && !(defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202311L)
typedef _Bool bool;
#endif
