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

SECTIONS
{
    /* ==== ROM ==== */
    .text : 
    { 
        /*  ----- Initialization Code ----- */
        *(.boot*)
        
        /* ----- Code ----- */
        /* Load all text sections (from all files) */
        *(.text)
        *(.text.*)
        
        . = ALIGN(4);

        /* ----- Read Only Data ----- */
        *(.rodata)
        *(.rodata.*)

        . = ALIGN(4);
         _etext = .;

    } > APPROM



    /* ==== RAM ==== */
    /* The .data section contains static variables which have an initial value at boot. */
    .data :
    {           
        _sdata = .;

        /* ----- Initialized Data ----- */       
        *(.data)
        *(.data.*)

        /* ----- Static Data ----- */
        . = ALIGN(16);
        _global_pointer = . + 0x800;

        *(.sdata)
        *(.sdata.*)
        *(.srodata.*)
        
        . = ALIGN(4);
        _edata = .;
    
    } > APPRAM AT> APPROM
    

    /* ----- Uninitialized Data ----- */
    /* .bss section which is used for uninitialized data */
    .bss (NOLOAD) :
    {   _sbss = .;
        *(.bss)
        *(.bss.*)
        *(COMMON)

        . = ALIGN(4);
        _ebss = .;
    
    } > APPRAM AT> APPROM

    _end = .;
}

PROVIDE(_start_heap = _ebss);
PROVIDE(_stack_pointer  = ORIGIN(APPRAM) + LENGTH(APPRAM));