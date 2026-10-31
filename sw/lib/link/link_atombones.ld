/*
    LINKER SCRIPT

    @See : https://sourceware.org/binutils/docs/ld/Basic-Script-Concepts.html
    @See : https://interrupt.memfault.com/blog/how-to-write-linker-scripts-for-firmware
    @See : https://github.com/pulp-platform/pulp-riscv-gnu-toolchain/blob/master/riscv.ld
*/
OUTPUT_FORMAT("elf32-littleriscv")
OUTPUT_ARCH( "riscv" )
ENTRY(_start)

/* MEMORY LAYOUT: ROM/RAM from the config (atom_config.h). This file is run through the C
   preprocessor by the libcatom build (into $(RVATOM_BUILD)/sw/lib/); do not pass it to ld directly. */
MEMORY
{
    CODE_RAM (rx):  ORIGIN = SOC_RAM_ADDR,                          LENGTH = SOC_RAM_SIZE - 16M
    DATA_RAM (rwx): ORIGIN = (ORIGIN(CODE_RAM) + LENGTH(CODE_RAM)), LENGTH = 16M
}

__coderam_start = ORIGIN(CODE_RAM);
__coderam_size  = LENGTH(CODE_RAM);
__dataram_start = ORIGIN(DATA_RAM);
__dataram_size  = LENGTH(DATA_RAM);

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

    } > CODE_RAM


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
    
    } > DATA_RAM AT> CODE_RAM
    

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
    
    } > DATA_RAM AT> CODE_RAM

    _end = .;
}

PROVIDE(_start_heap = _ebss);
PROVIDE(_stack_pointer  = ORIGIN(DATA_RAM) + LENGTH(DATA_RAM));