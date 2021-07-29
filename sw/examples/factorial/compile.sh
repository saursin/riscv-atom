riscv64-unknown-elf-gcc -mabi=ilp32 -march=rv32i -nostartfiles \
 ../../lib/startup.s ../../lib/stdio.c -T ../../lib/link.ld factorial.c -o factorial.elf $1 $2 $3 $4