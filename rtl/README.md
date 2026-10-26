# RTL

This directory contains the Verilog RTL for RISC-V Atom.

HydrogenSoC topmodule resides in the `HydrogenSoC.v` file.  
AtomBones topmodule resides in the `AtomBones.v` file.  

## Core
This subdirectory contains the verilog source files for RISC-V Atom core.  
The top-module resides in `AtomRV.v` file.

## Uncore
This subdirectory contains the verilog source files SoC peripherals like RAM, UART, SPI, GPIO, etc.  

## Lib
This subdirectory contains generic, reusable modules (FIFO, arbiter, priority encoder, IO buffer, etc.) that are
not specific to the core, a bus, or a peripheral. Shared headers live in `common/`.


