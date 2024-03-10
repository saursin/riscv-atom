/*
    LINKER SCRIPT

    @See : https://sourceware.org/binutils/docs/ld/Basic-Script-Concepts.html
    @See : https://interrupt.memfault.com/blog/how-to-write-linker-scripts-for-firmware
    @See : https://github.com/pulp-platform/pulp-riscv-gnu-toolchain/blob/master/riscv.ld
*/
OUTPUT_FORMAT("elf32-littleriscv")
OUTPUT_ARCH( "riscv" )
ENTRY(_start)

/* MEMORY LAYOUT */
MEMORY
{
    BOOT_ROM (rx):  ORIGIN = 0x00010000, LENGTH = 8K
    RAM (rx):       ORIGIN = 0x20000000, LENGTH = 48K
}

__bootrom_start = ORIGIN(BOOT_ROM);
__bootrom_size  = LENGTH(BOOT_ROM);
__approm_start  = ORIGIN(RAM);
__approm_size   = LENGTH(RAM);


SECTIONS
{
    /* ==== CODE RAM ==== */
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

    } > BOOT_ROM


    /* ==== DATA RAM ==== */
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
    
    } > RAM AT> BOOT_ROM
    

    /* ----- Uninitialized Data ----- */
    /* .bss section which is used for uninitialized data */
    .bss (NOLOAD) :
    {   _sbss = .;
        *(.sbss)
        *(.sbss.*)
        *(.bss)
        *(.bss.*)
        *(COMMON)

        . = ALIGN(4);
        _ebss = .;
    
    } > RAM AT> BOOT_ROM

    _end = .;
}

PROVIDE(_start_heap = _ebss);
PROVIDE(_stack_pointer  = ORIGIN(RAM) + LENGTH(RAM));