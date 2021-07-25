/*
    LINKER SCRIPT

    @See : https://sourceware.org/binutils/docs/ld/Basic-Script-Concepts.html
    @See : https://interrupt.memfault.com/blog/how-to-write-linker-scripts-for-firmware
*/

OUTPUT_ARCH( "riscv" )
ENTRY(_start)

/* MEMORY LAYOUT */
MEMORY
{
    ROM (x!r) : ORIGIN = 0x00000000, LENGTH = 32K    /* 32 KB @ 0x0*/
    RAM (rwx): ORIGIN = 0x04000000, LENGTH = 8K    /* 8 KB @ 0x04000000 (0.5GB boundry)*/
}

SECTIONS
{
    /* ==== ROM ==== */
    .text : 
    { 
        *(.boot*)
        . = ORIGIN(ROM) + 0x100;    /* 256 Bytes (64 Instructions)*/
        
        _svector = .;
        KEEP(*(.vector*))   /* Keep all interrupt vector tables at very start of text section */

        *(.text*)            /* Load all text sections (from all files) */
        . = ALIGN(4);
        
        _etext = .;
    } > ROM



    /* ==== RAM ==== */
    /* The .data section contains static variables which have an initial value at boot. */
    .rodata :
    {
        *(.rodata*)
    } > RAM

    .data :
    {
        _sdata = .;
        *(.data*)
        _global_pointer = . + 0x800;

        . = ALIGN(4);
        _edata = .;
    } > RAM /*> ram AT >rom*/

    .sdata :
    {
        *(.sdata*)
    } > RAM

    /* .bss section which is used for uninitialized data */
    .bss (NOLOAD) :
    {   _sbss = .;
        *(.bss*)
        *(COMMON)

        . = ALIGN(4);
        _ebss = .;
    } > RAM

    _end = . ;
}

PROVIDE(_start_heap = _ebss);
PROVIDE(_stack_pointer  = ORIGIN(RAM) + LENGTH(RAM));