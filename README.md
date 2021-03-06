![RISCVAtom-header](docs/diagrams/rvatom-header.drawio.png)

# RISCV-Atom

[![MIT license](https://img.shields.io/badge/License-MIT-blue.svg)](https://lbesson.mit-license.org/) [![atomsim-atombones](https://github.com/saurabhsingh99100/riscv-atom/actions/workflows/atomsim-atombones.yml/badge.svg)](https://github.com/saurabhsingh99100/riscv-atom/actions/workflows/atomsim-atombones.yml) [![atomsim-hydrogensoc](https://github.com/saurabhsingh99100/riscv-atom/actions/workflows/atomsim-hydrogensoc.yml/badge.svg)](https://github.com/saurabhsingh99100/riscv-atom/actions/workflows/atomsim-hydrogensoc.yml) [![Documentation Status](https://readthedocs.org/projects/riscv-atom/badge/?version=latest)](https://riscv-atom.readthedocs.io/en/latest/?badge=latest)


Atom is a 32-bit embedded-class processor based on the [RISC-V](https://riscv.org/) Instruction Set Architecture (ISA). It is designed to be light on hardware resources and still powerful enough to be deployed in most embedded class applications.

Key highlights of Atom are listed below:

- Implements RV32I + *M(optional) ISA as defined in the [RISC-V unprivileged ISA manual](https://github.com/riscv/riscv-isa-manual/releases/download/Ratified-IMAFDQC/riscv-spec-20191213.pdf).

- Simple 2-stage pipelined architecture, ideal for smaller FPGAs.

- Wishbone ready CPU interface.

- Interactive RTL simulator AtomSim.

- Wide range of example programs.

- Open source under [MIT License](https://en.wikipedia.org/wiki/MIT_License).  

***To checkout this project, Please refer to the Getting Started Guide***: [link](https://riscv-atom.readthedocs.io/en/latest/pages/getting-started/riscv-atom.html)

*To be added soon

## Directory Structure

- `build` : Build files **(autogenerated)**
  - `bin` : executable binaries
  - `cobj_dir` : g++ generated object files
  - `vobj_dir` : verilator generated object files
  - `dump` : memory dumps
  - `init` : verilog memory initialization files
  - `trace` : VCD traces

- `docs` : RISCV-Atom documentation: [readthedocs](https://riscv-atom.readthedocs.io/en/latest/index.html)

- `rtl` : Verilog sources
  - `core` : RISCV-Atom core components
  - `uncore` : RISCV-Atom non-core components (peripherals)
  - `dpi` : SystemVerilog DPI sources

- `scripts` : scripts for commonly used commands (ConvELF.py resides here)

- `sim` : Atomsim source code
  - `docs` : Doxygen Documentation for Atomsim source code

- `sw` : RISCV-Atom software sources
  - `examples` : RISCV-Atom examples
  - `lib` : libc for RISCV-Atom *(libcatom)*
    - `include` : libcatom headers
    - `libcatom` : libcatom sources
    - `link` : RISC-V Atom linker scripts

- `synth` :
  - `xilinx`: Synthesis project for xilinx FPGAs
  - `yosys` : Yosys synthesis scripts

- `test` : RISCV Atom tests
  - `riscv-target` : Official RISC-V compliance test files
  - `scar` : SCAR tests directory

- `tools` : utility tools
  - `elfdump` : elfdump utility


## Useful Links
1. [Project Website](https://sites.google.com/view/saurabh-singh-web/projects/risc-v-atom?authuser=0)
2. [Getting Started Guide](https://riscv-atom.readthedocs.io/en/latest/pages/getting-started/prerequisites.html)
3. [Examples Guide](https://riscv-atom.readthedocs.io/en/latest/pages/getting-started/examples.html)
4. [Project Documentation](https://riscv-atom.readthedocs.io/en/latest/index.html)
5. AtomSim Source Documentation


## Other related projects
1. **AtomShell**: A simple shell for RISC-V Atom based SoCs : [AtomShell Github](https://github.com/saurabhsingh99100/atomshell)


## License
This project is open-source under MIT license!
