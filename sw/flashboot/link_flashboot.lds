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
    BOOTROM (rx) : ORIGIN = 0x00010000, LENGTH = 3K
    BOOTRAM (rwx): ORIGIN = 0x00010c00, LENGTH = 1K

    APPROM  (rx) : ORIGIN = 0x00011000, LENGTH = 28K
    APPRAM  (rwx): ORIGIN = 0x20000000, LENGTH = 16K
}

__bootrom_start = ORIGIN(BOOTROM);
__bootrom_size  = LENGTH(BOOTROM);

__approm_start  = ORIGIN(APPROM);
__approm_size   = LENGTH(APPROM);

SECTIONS
{
    /* ==== ROM ==== */
    .text : 
    { 
        /*  ----- Initialization Code ----- */
        *(.boot*)
        *(.text)
        *(.text.*)
        *(.rodata)
        *(.rodata.*)
        . = ALIGN(4);
        
    } > BOOTROM



    /* ==== RAM ==== */
    /* The .data section contains static variables which have an initial value at boot. */
    .data :
    {
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
    } > BOOTRAM
    
    /* ----- Uninitialized Data ----- */
    /* .bss section which is used for uninitialized data */
    .bss (NOLOAD) :
    {   _sbss = .;
        *(.bss)
        *(.bss.*)
        *(COMMON)

        . = ALIGN(4);
        _ebss = .;
    } > BOOTRAM

    _end = . ;
}

PROVIDE(_start_heap = _ebss);
PROVIDE(_stack_pointer  = ORIGIN(BOOTRAM) + LENGTH(BOOTRAM));