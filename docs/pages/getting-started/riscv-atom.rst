*********************
RISC-V Atom Overview
*********************
.. image:: https://img.shields.io/badge/License-MIT-blue.svg
   :target: https://github.com/saurabhsingh99100/riscv-atom/blob/main/LICENSE
   :alt: License
   
Atom is a 32-bit embedded-class softcore processor based on the RISC-V Instruction Set Architecture (ISA).
It is designed to be light on hardware resources and still powerful enough to be deployed in most embedded
class applications.

Key highlights of Atom are listed below:

- 32-bit RISC-V ISA CPU core.
- Supports RV32I + M(optional) ISA as defined in the RISC-V unprivileged ISA manual.
- Simple 2-stage pipelined architecture, ideal for smaller FPGAs.
- Only machine mode privilege level supported.
- Wishbone ready CPU interface.
- Advanced Multi-Target Interactive RTL simulator: AtomSim.
- Fully Compliant with riscv-arch-tests.
- Open source under MIT License.


.. image:: ../../diagrams/atomRV_architecture_diagram.png
