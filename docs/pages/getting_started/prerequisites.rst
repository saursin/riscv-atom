Prerequisites
###############
This page discusses how to set up your system in order to get RISC-V Atom up and running.

Required Packages
******************

.. note::
  RISC-V Atom project has been developed and tested on **Ubuntu 20.04**.
  However, It should work just fine on any other linux based distribution with relevant packages.

Install git, make, python3, gcc & other tools
==============================================
RISC-V Atom uses *Make* for all builds. *GNU C/C++ compilers*, *Make* and other essential build tools are conveniently
packaged as ``build-essential`` meta package in Ubuntu. RISC-V Atom uses the GNU Readline library to implement the
interactive console in AtomSim. We also want to install GTKWave to view VCD waveforms and screen to connect to serial ports.

.. code-block:: bash

  $ sudo apt-get update
  $ sudo apt-get install git python3 python3-pip build-essential libreadline8 libreadline-dev socat
  $ sudo apt-get install gtkwave screen
  $ pip install -r requirements.txt


Install Verilator
==================
Verilator will be used By AtomSim to *Verilate* Verilog RTL into C++. We recommend installing the latest stable
Verilator version (>=5.034) using `git quick install method <https://veripool.org/guide/latest/install.html>`_.


Install RISC-V GNU Toolchain
=============================
We will be installing the **RISC-V 64-bit Multilib** toolchain
Further install instructions can be found `here <https://github.com/riscv-collab/riscv-gnu-toolchain>`_. 
We recommend using the provided ``install_toolchain.sh`` script to install the proper toolchain.

.. code-block:: bash

  $ chmod +x install_toolchain.sh
  $ ./install_toolchain.sh

Allow user to access serial ports
=================================
To allow current linux user to access serial ports and usb devices (such as JTAG), the user must be added to 
the ``dialout`` and ``plugdev`` groups respectively.

.. code-block:: bash

  $ sudo usermod -aG dialout $USER
  $ sudo usermod -aG plugdev $USER

.. note::
  This takes effect after user logs out and logs back in. 

OpenFPGAloader
==============
We use openFPGAloader to load bitstreams on FPGA. you are free to use vendor tools instead. To install openFPGAloader
follow `this <https://trabucayre.github.io/openFPGALoader/>`_ guide.

----------------

Optional Packages
******************

.. note::
  The following packages are optional and are only required for generating documentation using 
  Doxygen & Sphinx

Install Doxygen
================
Doxygen a tool is used to generate C++ source code documentation from comments inside the C++ source files.

.. code-block:: bash

  $ sudo apt-get install doxygen


Install Latex Related packages
===============================
These packages are essential for generating Latex documentation using Doxygen.

.. code-block:: bash

  $ sudo apt -y install texlive-latex-recommended texlive-pictures texlive-latex-extra latexmk


Install sphinx & other python dependencies
===========================================
Sphinx is used to generate the RISC-V Atom Documentation and User-Manual in PDF & HTML. To install the packages to
generate sphinx documentation, run the following command in ``docs`` directory under riscv-atom repository.

.. code-block:: bash

  $ pip install -r docs/requirements.txt
  $ sudo apt install graphviz             # Graphviz is for flow diagrams in sphinx documentation
