***************
Prerequisites
***************
This section discusses how to set up your system in order to get riscv-atom up and running.
Atom has been developed and tested on ubuntu 20.04. It should work just fine on any other version of 
ubuntu with no or few additional packages. 


Run apt update
================
::

  $ sudo apt update

Install git, make, gcc & other tools
=====================================
::

  $ sudo apt install git build-essential

Install Verilator
==================
::

  $ sudo apt install verilator

Install GTK Wave
==================
::

  $ sudo apt install gtkwave


Install RISC-V GNU Toolchain
=============================
You can either install 
1). a RV32I toolchain explicitly or 
2). a RV64-Multilib toolchain.

In this tutorial we'll be going with the second option.
If we go with the second option we will have to supply `-march=rv32i`, `-mabi=ilp32` arguments explicitly
whenever we want to use the tools.

Further install instructions can be found `here <https://github.com/riscv/riscv-gnu-toolchain>`_
  
  
Install RISC-V tools
====================
Install instructions can be found `here <https://github.com/riscv/riscv-tools>`_


.. note:: The following packages are optional and are only required for generating documentation using doxygen & sphinx


Install Doxygen
================
::

  $ sudo apt install doxygen

Install texlive packages
=========================
::

  $ sudo apt -y install texlive-latex-recommended texlive-pictures texlive-latex-extra


Install sphinx
===============
::

  $ pip install sphinx

Install latexmk
================
::

  $ sudo apt install latexmk

