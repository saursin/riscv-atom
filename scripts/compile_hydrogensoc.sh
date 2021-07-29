#!/bin/bash
#!/bin/bash
RVPREFIX=riscv64-unknown-elf
CC=$RVPREFIX-gcc

$CC -mabi=ilp32 -march=rv32i -nostartfiles $RVATOM/sw/lib/startup.s $RVATOM/sw/lib/stdio.c \
-T $RVATOM/sw/lib/link_hydrogensoc.ld $*