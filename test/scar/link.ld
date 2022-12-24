/*
    SCAR LINKER SCRIPT

    @See : https://sourceware.org/binutils/docs/ld/Basic-Script-Concepts.html
    @See : https://interrupt.memfault.com/blog/how-to-write-linker-scripts-for-firmware
*/

OUTPUT_ARCH( "riscv" )
ENTRY(_start)

/* MEMORY LAYOUT */
MEMORY
{
    ROM (rx) : ORIGIN = 0x00010000, LENGTH = 64M    /* 64 MB @ 0x0*/
    RAM (rwx): ORIGIN = 0x20000000, LENGTH = 64M    /* 64 MB @ 0x10000 (0x04000000)*/
}

SECTIONS
{
    /* ==== ROM ==== */
    .text : 
    {
        *(.text)            /* Load all text sections (from all files) */
        *(.rodata)

        . = ALIGN(4);
        _etext = .;
    } > ROM



    /* ==== RAM ==== */
    /* The .data section contains static variables which have an initial value at boot. */
    /* .data :
    {
        _sdata = .;
        *(.data*)
        _global_pointer = . + 0x800;

        . = ALIGN(4);
        _edata = .;
    } > RAM > ram AT >rom */

    /* .bss section which is used for uninitialized data
    .bss (NOLOAD) :
    {   _sbss = .;
        *(.bss*)
        *(COMMON)

        . = ALIGN(4);
        _ebss = .;
    } > RAM */

    _end = . ;
}

PROVIDE(_start_heap = _ebss);
PROVIDE(_stack_pointer  = ORIGIN(RAM) + LENGTH(RAM));