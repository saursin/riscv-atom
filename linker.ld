/*
    LINKER SCRIPT
*/

/*
@ See : https://sourceware.org/binutils/docs/ld/Basic-Script-Concepts.html
@ See : https://interrupt.memfault.com/blog/how-to-write-linker-scripts-for-firmware

The ‘SECTIONS’ command is a powerful command. Here we will describe a simple use of it. 
Let’s assume your program consists only of code, initialized data, and uninitialized data. 
These will be in the ‘.text’, ‘.data’, and ‘.bss’ sections, respectively. 
Let’s assume further that these are the only sections which appear in your input files.

For this example, let’s say that the code should be loaded at address 0x10000, and that 
the data should start at address 0x8000000. Here is a linker script which will do that:

*/

MEMORY
{
    rom (rx) : ORIGIN = 0x00000000, LENGTH = 0x0000ffff /* 64 kb @ 0x0*/
    ram (rwx): ORIGIN = 0x00010000, LENGTH = 0x0000ffff /* 64 kb @ 0xffff (65535)*/
}

SECTIONS
{
    . = 0x32;
    .text : 
    { 
        *(.text) 
    } > rom


    . = 0x40;
    .data : 
    { 
        *(.data) 
    } > ram
  
    .bss : 
    {
        *(.bss)
    } > ram
}