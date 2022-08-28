Prerequisites
###############

This page discusses how to set up your system in order to get riscv-atom up and running.

Required Packages
******************

.. note::
  RISC-V Atom project has been developed and tested on ubuntu 20.04. 
  However, It should work just fine on any other version of ubuntu with no or few additional packages. 


Run apt update
================
This step is needed to make sure the apt package list is up to date.

.. code-block:: bash

  $ sudo apt update


Install git, make, python3, gcc & other tools
==============================================
GNU C/C++ compilers and Make and other essential build tools are conveniently packaged as ``build-essential`` meta package.

.. code-block:: bash

  $ sudo apt install git python3 build-essential


Install Verilator
==================
Verilator will be used By Atomsim to *Verilate* Verilog RTL into C++.

.. code-block:: bash

  $ sudo apt install verilator


Install GTK Wave
==================
GTKwave is a GUI tool to view waveforms stored as Value Change Dump (VCD) files.

.. code-block:: bash

  $ sudo apt install gtkwave


Install Screen
==================
Screen is a command line utility that can be used to connect to serial ports on linux. 
It will be used to estabilish a two-way serial communication with the AtomSim.

.. code-block:: bash

  $ sudo apt install screen


Install RISC-V GNU Toolchain
=============================
We will be installing the RV64-Multilib toolchain
Further install instructions can be found `here <https://github.com/riscv-collab/riscv-gnu-toolchain>`_. 
We recommend using the provided ``install_toolchain.sh`` script to install the proper toolchain.

.. code-block:: bash

  $ chmod +x install_toolchain.sh
  $ ./install_toolchain.sh


----------------

|

Optional Packages
******************

.. note:: The following packages are optional and are only required for generating documentation using doxygen & sphinx

Install Doxygen
================
Doxygen a tool is used to generate C++ cource code documentation from =documentation comments= inside the C++ source files.

.. code-block:: bash

  $ sudo apt install doxygen

Install Latex Related packages
===============================
These packages are essential for generating Latex documentation using Doxygen.

.. code-block:: bash

  $ sudo apt -y install texlive-latex-recommended texlive-pictures texlive-latex-extra latexmk


Install sphinx & other python dependencies
===========================================
Sphinx is used to generate the RISC-V Atom Documentation and User-Manual in PDF & HTML.

.. code-block:: bash

  $ cd docs/ && pip install -r requirements.txt


