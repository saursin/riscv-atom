# riscv-atom

Atom is an embedded-class processor based on the open-source 32-bit RISC-V instruction set architecture.

- Atom has a non pipe-lined architecture, optimised for small FPGAs.
- Atom Implements fully implements RV32I  instruction set architecture.

## Table of Contents

1. [Atom Architecture](Atom-Architecture)
2. [Atom SoC](Atom-SoC)
3. [Prerequisites](Prerequisites)
4. [Getting Started](Getting-Started)
5. [Simulation](Simulation)
6. [FPGA Implementation](FPGA-Implementation)
7. [License](License)



## Atom Architecture



## Atom SoC



## Prerequisites

Install verilator

````bash
$ sudo apt install verilator
````



## Getting Started

Get [RISC-V tools](https://github.com/riscv/riscv-tools) & [RISC-V GNU Toolchain](https://github.com/riscv/riscv-gnu-toolchain.git) . Make sure that your `RISCV` environment variable points to your RISC-V installation (see the RISC-V tools and related projects for further information).

**Clone the repository**

```bash
$ git clone https://github.com/saurabhsingh99100/riscv-atom.git
```



## Simulation

Run make from the parent directory

```bash
$ make sim
```

This will create a build directory inside the parent directory. The simulation executable will be available here.

```bash
$ cd build
$ ./sim hello.elf
```



## FPGA Implementation



## License

Atom is open-source under the MIT license! 

Click [here](LICENSE) to know more.
