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
    BOOT_ROM (rx): ORIGIN = 0x00010000, LENGTH = 4K
    CODE_RAM (rx): ORIGIN = 0x20000000, LENGTH = 32K
    DATA_RAM (rw): ORIGIN = 0x20008000, LENGTH = 16K
}

__bootrom_start = ORIGIN(BOOT_ROM);
__bootrom_size  = LENGTH(BOOT_ROM);
__approm_start  = ORIGIN(CODE_RAM);
__approm_size   = LENGTH(CODE_RAM);


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
    
    } > DATA_RAM AT> BOOT_ROM
    

    /* ----- Uninitialized Data ----- */
    /* .bss section which is used for uninitialized data */
    .bss (NOLOAD) :
    {   _sbss = .;
        *(.bss)
        *(.bss.*)
        *(COMMON)

        . = ALIGN(4);
        _ebss = .;
    
    } > DATA_RAM AT> BOOT_ROM

    _end = .;
}

PROVIDE(_start_heap = _ebss);
PROVIDE(_stack_pointer  = ORIGIN(DATA_RAM) + LENGTH(DATA_RAM));