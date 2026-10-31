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
    BOOT_ROM (rx):  ORIGIN = SOC_BOOTROM_ADDR, LENGTH = SOC_BOOTROM_SIZE
    RAM (rx):       ORIGIN = SOC_RAM_ADDR,     LENGTH = SOC_RAM_SIZE
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
    
    /* Reserve for Approm */
    .appram_reserved (NOLOAD) : {
        . = . + LENGTH(RAM) - 2K;   /* Use last 2K of RAM for bootloader stack and data */
        . = ALIGN(16);
    } > RAM AT> BOOT_ROM

    /* Bootrom data section */
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