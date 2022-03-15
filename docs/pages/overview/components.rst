Components
###########

This page gives a high-level overview of various components of the RISC-V Atom project.

RISC-V Atom Core
*****************

Atom is a 32-bit embedded-class softcore processor written in Verilog HDL. It is designed to cater to embedded class applications. It is fully compliant with the open-source RISC-V Instruction Set Architecture (RV32I) and passes all official RISC-V compliance tests. Atom is based on a two-stage pipelined architecture inspired by the ARM cortex m0+ processor. It is aimed towards implementation on FPGAs. 


.. link-button:: RISC-V Atom (Core)
    :type: ref
    :text: Click here for in-depth documentation
    :classes: btn-outline-primary btn-block


SoC Targets
************

* **AtomBones**: It is a stub-target that consists of a single atom core only. The instruction memory, data memory, and serial port are simulated in C++. It is should be used for simulation and debugging purposes only.

* **HydrogenSoC**: It is a basic SoC implementation that contains a single Atom core along with instruction memory, data memory, serial ports, GPIO pins, etc. All the peripherals are connected with the CPU using a Wishbone-B4 bus.

.. link-button:: RISC-V Atom SoC Targets
    :type: ref
    :text: Click here for in-depth documentation
    :classes: btn-outline-primary btn-block


AtomSim
********

AtomSim is a Spike-like simulator written in C++. In contrast to Spike, AtomSim Simulates the actual RTL in the backend. AtomSim provides a hardware-software co-simulation environment for developing applications and extensions on the Atom platform. It uses the Verilator tool to compile the RTL into a shared object which then gets linked with the C++ based simulator frontend to simulate the system. AtomSim also features a command-line interface to control various aspects of the simulation like start, stop, step, run indefinitely, run for a specified number of cycles, enable/disable vcd tracing, etc. It also supports run-time instruction disassembly with the help of the RISC-V objdump tool.

.. link-button:: AtomSim: A simulation tool for Atom based SoCs
    :type: ref
    :text: Click here for in-depth documentation
    :classes: btn-outline-primary btn-block


SCAR
*****

SCAR (Search, Compile Assert, and Run) is a processor verification framework written in python. It performs a set of assembly-level tests to verify the processor implementation. Each assembly test usually checks for one particular function of the processor. SCAR does this by examining a state dump after the processor is done with executing a test code. This state dump is then checked assuming a set of assertions in the form of expected register values. These assertions are provided in the assembly file itself. SCAR is also used to verify the RISC-V ISA compliance in this project.

.. link-button:: SCAR: Search Compile Assert Run
    :type: ref
    :text: Click here for in-depth documentation
    :classes: btn-outline-primary btn-block


ConvELF
********

ConvELF is a python script that is used to convert an ELF executable file to verilog friendly memory initialization files. These files can be in either `hex` or `bin` format and can be used to initialize a verilog memory with the help of `$readmemh` & `readmemb` functions. ConvELF is a flexible program configured by a memory map specified as a python dictionary. It can even fragment an elf file into multiple memory initialization files depending on the memory map provided.

.. link-button:: ConvELF: A Utility Tool for ELF Conversion
    :type: ref
    :text: Click here for in-depth documentation
    :classes: btn-outline-primary btn-block
