/*
    LINKER SCRIPT

    @See : https://sourceware.org/binutils/docs/ld/Basic-Script-Concepts.html
    @See : https://interrupt.memfault.com/blog/how-to-write-linker-scripts-for-firmware
    @See : https://github.com/pulp-platform/pulp-riscv-gnu-toolchain/blob/master/riscv.ld
*/

OUTPUT_ARCH( "riscv" )
ENTRY(_start)

/* MEMORY LAYOUT */
MEMORY
{
    ROM (rx) : ORIGIN = 0x00000000, LENGTH = 32K    /* 32 KB @ 0x0 */
    RAM (rwx): ORIGIN = 0x04000000, LENGTH = 8K    /* 8 KB @ 0x04000000 (0.5GB boundry)*/
}

SECTIONS
{
    /* ==== ROM ==== */
    .text : 
    { 
        /*  ----- Initialization Code ----- */
        *(.boot*)
        . = ORIGIN(ROM) + 0x100;    /* 256 Bytes (64 Instructions)*/
        
        /* ----- Interrupt Vector Table ----- */
        _svector = .;
        KEEP(*(.vector*))

        /* ----- Code ----- */
        /* Load all text sections (from all files) */
        *(.text)
        *(.text.*)
        
        . = ALIGN(4);

        _etext = .;
    } > ROM



    /* ==== RAM ==== */
    /* The .data section contains static variables which have an initial value at boot. */
    .data :
    {
        /* ----- Read Only Data ----- */
        *(.rodata)
        *(.rodata.*)
        
        . = ALIGN(4);
        
        
        /* ----- Initialized Data ----- */
        _sdata = .;
        
        *(.data)
        *(.data.*)

        . = ALIGN(4);
        _edata = .;


        /* ----- Static Data ----- */
        . = ALIGN(16);
        _global_pointer = . + 0x800;

        *(.sdata)
        *(.sdata.*)
        *(.srodata.*)
        
        . = ALIGN(4);
    } > RAM

    
    /* ----- Uninitialized Data ----- */
    /* .bss section which is used for uninitialized data */
    .bss (NOLOAD) :
    {   _sbss = .;
        *(.bss)
        *(.bss.*)
        *(COMMON)

        . = ALIGN(4);
        _ebss = .;
    } > RAM

    _end = . ;
}

PROVIDE(_start_heap = _ebss);
PROVIDE(_stack_pointer  = ORIGIN(RAM) + LENGTH(RAM));