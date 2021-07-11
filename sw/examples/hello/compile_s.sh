riscv64-unknown-elf-gcc -mabi=ilp32 -march=rv32i -nostartfiles \
 ../../lib/startup.s -T ../../lib/link.ld hello.s -o hello.elf