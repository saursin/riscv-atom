.. image:: ../../diagrams/rvatom-header.drawio.png

|

.. image:: https://img.shields.io/badge/License-MIT-blue.svg
   :target: https://github.com/saurabhsingh99100/riscv-atom/blob/main/LICENSE
   :alt: License

|


Introduction
##############

**RISC-V Atom** is an open-source soft-core processor platform targeted for FPGAs. It is complete hardware prototyping 
and software development environment based around **Atom**, which is a 32-bit embedded-class processor based on the 
`RISC-V <https://riscv.org/>`_ Instruction Set Architecture (ISA). 

Key Highlights 
***************
Key highlights of the RISC-V Atom projects are are listed below:

#. Atom implements ``RV32IC_Zicsr`` ISA as defined in the `RISC-V unprivileged ISA manual <https://github.com/riscv/riscv-isa-manual/releases/download/Ratified-IMAFDQC/riscv-spec-20191213.pdf>`_.
#. Simple 2-stage pipelined architecture, ideal for smaller FPGAs.
#. Optional support for RISC-V exceptions and interrupts.
#. Wishbone ready CPU interface.
#. Interactive RTL simulator - :doc:`AtomSim</pages/documentation/atomsim/atomsim>`.
#. In-house verification framework - :doc:`SCAR</pages/documentation/scar>`.
#. Multiple SoC configurations.
#. Tiny libc like standard library - **libcatom**.
#. Wide range of example programs.
#. Open source under `MIT License <https://en.wikipedia.org/wiki/MIT_License>`_.

.. tip::
   To get started, Check out the :doc:`getting started guide</pages/getting_started/prerequisites>`.


Components
***********
Following is list of various components of the RISC-V Atom project.

.. card:: RISC-V Atom CPU
   :link-type: doc
   :link: /pages/documentation/riscv_atom/riscv_atom

   A simple 32-bit RISC-V processor.

.. card:: SoC Targets
   :link-type: doc
   :link: /pages/documentation/soc_targets

   RISC-V Atom project provides several configurable SoC targets that can be built around the Atom CPU.

.. card:: AtomSim
   :link-type: doc
   :link: /pages/documentation/atomsim/atomsim

   AtomSim is the interactive RTL simulator for RISC-V Atom SoCs.

.. card:: SCAR
   :link-type: doc
   :link: /pages/documentation/scar

   SCAR (Search, Compile Assert, and Run) is an in-house processor verification framework written in python.
