////////////////////////////////////////////////////////////////////   
//  File        : AtomBones_Config.vh
//  Author      : Saurabh Singh (saurabh.s99100@gmail.com)
//  Description : Configuration file for AtomBones. It defines the 
//      AtomBones related parameters using verilog preprocessor macros.
////////////////////////////////////////////////////////////////////
`ifndef __ATOMBONES_CONFIG_VH__
`define __ATOMBONES_CONFIG_VH__

////////////////////////////// SoC Config Macros //////////////////////////////
/*
    Reset address for SOC. Upon Reset, PC jumps to the defined address
*/
`ifndef SOC_RESET_ADDRESS
`define SOC_RESET_ADDRESS   32'h0001_0000
`endif

/*
    If defined, enables the RVE extenstion which disables the upper 16 registers 
    in the register file.
*/
`ifdef ENABLE_RISCV_EMBEDDED
`define RV_E
`endif

/*
    If defined, enables the RISC-V Zicsr extension which adds support for CSR 
    registers. It adds a CSR Unit to the Core which implements CSR registers 
    like CYCLEL/H.
*/
`ifdef ENABLE_RISCV_ZICSR
`define RV_ZICSR
`endif

/*
    If defined, enables the RISC-V Compressed extension which adds support for 16-bit 
    compressed instructions. It adds a decoder which decodes the 16 bit instructions 
    to 32 bit equivalents and a aligner to word-align Instruction fetches.
*/
`ifdef ENABLE_RISCV_COMPRESSED
`define RV_C
`endif

/*
    If defined enables exceptions and interrupts
*/
`ifdef ENABLE_RISCV_EXCEPTIONS
`define RV_ZICSR
`define EN_EXCEPT
`endif

`endif //__ATOMBONES_CONFIG_VH__
