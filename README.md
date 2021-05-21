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



## AtomRV Architecture



## AtomRVSoC



## Prerequisites

Run apt update

```bash
$ sudo apt update
```

Install git, make, gcc & other tools

```bash
$ sudo apt-get install git build-essential
```

Install verilator

````bash
$ sudo apt install verilator
````

Install GTK Wave

```bash
$ sudo apt-get install gtkwave
```



**Note : ** The following packages are optional and are only required for generating documentation using doxygen

Install doxygen

```bash
$ sudo apt install doxygen
```

Install texlive packages (Only needed for [pdf generation](Getting-Started))

```bash
$ sudo apt-get -y install texlive-latex-recommended texlive-pictures texlive-latex-extra
```



## Getting Started

Get [RISC-V tools](https://github.com/riscv/riscv-tools) & [RISC-V GNU Toolchain](https://github.com/riscv/riscv-gnu-toolchain.git) . Make sure that your `RISCV` environment variable points to your RISC-V installation (see the RISC-V tools and related projects for further information).

**Clone the repository**

```bash
$ git clone https://github.com/saurabhsingh99100/riscv-atom.git
$ cd riscv-atom
```

**Building the Simulator**

Run make from the parent directory

```bash
$ make atomsim
```

**Generating documentation**

Run make from the parent directory

```bash
$ make doc
```

This will generate *latex* and *html* documentation in their respective folders under the doc directory

**PDF Generation**

To generate *pdf* file from the latex files, go to `doc/latex` directory and run `make`



## AtomSim

### CLI Options

**UPDATE NEEDED**

```
Usage:
$ atomsim [options] <executable_file>

options:
--version			: display version info
-d					: debug/interactive mode
-l <file> 			: log all outputs
--extension=<name>	: Specify RoCC Extension
```

### Modes of operation

#### 1. Static Mode

To invoke atomsim in static mode, invoke Charon as follows:

```
$ atomsim hello.elf
atomsim v1 : [hello] 
running...
finished at tick 550103
```

#### 2. Interactive Mode

To invoke interactive debug mode, launch Charon with '-d' flag:

```
$ atomsim -d hello
Charon v1 : [hello] : ready...
:
```

##### Displaying contents of a register

```
: getr <reg>
```

- where reg is the ABI name for the register to be displayed. ex `: getr r0`, `: getr lr`, `: getr a0`

- use ':' to display a range of registers. ex `: getr r0:r7`

- use 'a' to display all registers. ex `: getr a`

  

##### Displaying Contents of a memory location

```
: getm <address>
```

- where address is the address of memory location.

- use ':' to display contents of memory in a range. ex `: getm 0x53:0x66`

  

##### Setting value of a register

```
: setr <reg> <value>
```

- where \<reg> is the ABI name for the register to be displayed. ex `: setr r0 56`, `: setr lr 0x550`, `: setr a0 21`

  

##### Controlling execution

You can advance by one instruction by pressing the ***enter-key***. You can also execute until a desired equality is reached:

- until value of a register \<reg> becomes \<value>

  ```
  : until <reg> <value>
  ```

- until value of a memory address \<address> becomes \<value>

  ```
  : until <address> <value>
  ```

- while \<condition> is true

  ```
  : while <condition>
  ```

- Execute for specified number of ticks

  ```
  : for <ticks>
  ```

- You can continue execution indefinitely by:

  ```
  : r
  ```

- To end the simulation from the debug prompt, press `<control>-<c>` or:

  ```
  : q
  ```

> At any point during execution (even without -d), you can enter the interactive debug mode with `<control>-<c>`.













This will create a build directory inside the parent directory. The simulation executable will be available here.

```bash
$ cd build
$ ./sim hello.elf
```



## FPGA Implementation

| FPGA                     | LUT Utilization | Power Consumption | Fmax |
| ------------------------ | --------------- | ----------------- | ---- |
| Xilinx Spartan-6 XC6SLX9 | -               | -                 | -    |
| Xilinx Atrix-7           | -               | -                 | -    |



## License

Atom is open-source under the MIT license! 

Click [here](LICENSE) to know more.
