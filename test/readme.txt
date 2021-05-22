To compile an assembly test program use 

    $ riscv64-unknown-elf-gcc -march=rv32i -mabi=ilp32 -O3 -nostdlib test/lui.s -o test/lui


To display disassembled contents of generated elf file use:

    $ riscv64-unknown-elf-objdump -d test/lui

To invoke the simulator:
    
    $ ./build/bin/atomsim -d --trace-dir . test/lui -v

    -d : open in debug mode (interactive)
    --trace-dir : specify a directory to store vcd traces
    -v verbose :
