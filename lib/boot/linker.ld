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
    rom (rx) : ORIGIN = 0x00000000, LENGTH = 0x0000ffff /* 64 kb @ 0x0*/
    ram (rwx): ORIGIN = 0x00010000, LENGTH = 0x00002000 /* 8 kb @ 0xffff (65535)*/
}

STACK_SIZE = 0x1000; /* 4kb */

SECTIONS
{
    /* ==== ROM ==== */
    .text : 
    { 
        KEEP(*(.vector*))   /* Keep all interrupt vector tables at very start of text section */
        *(.text)            /* Load all text sections (from all files) */
        *(.rodata)
        _etext = .;
    } > rom



    /* ==== RAM ==== */
    /* The .data section contains static variables which have an initial value at boot. */
    .data :
    {
        __sdata = .;
        *(.data*)
        __edata = .;
    } > ram /*> ram AT >rom*/


    /* stack section */
    .stack (NOLOAD):
    {
        . = ALIGN(16);
        . = . + STACK_SIZE;
        . = ALIGN(16);
    } > ram

    /* .bss section which is used for uninitialized data */
    .bss (NOLOAD) :
    {   __sbss = .;
        *(.bss*)
        *(COMMON)
        __ebss = .;
    } > ram

    _end = . ;
}
