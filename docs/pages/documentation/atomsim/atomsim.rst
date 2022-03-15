AtomSim: A simulation tool for Atom based SoCs
###############################################

AtomSim is an interactive RTL simulator for Atom based SoCs. It provides an interface which is similar to the RISC-V Spike simulator, but simulates the actual RTL in the backend. Atomsim is a modular and flexible simulaion solution based on Verilator, due to which it can achieve a very high simulation rate. AtomSim is a feature rich tool which makes it very powerful for debugging code on the Atom CPU.

Key Features of AtomSim are listed below:

#. Achieves a high simulation rate due to use of Verilator.
#. Target Configurable, can be easily extended for new SoC designs.
#. In-built debug mode similar to spike.
#. External Debug Support using OpenOCD & GDB ``[TODO]``.
#. Supports VCD trace generation.
#. Supports memory dumps.
#. Compatible with RISC-V compliance tests framework.
#. Compatible with SCAR framework.

To view available command line options, use:

.. code-block:: bash

  $ atomsim --help


Modes of Operation
*******************
Atomsim supports two modes of operation:

.. toctree::
   :maxdepth: 1

   debug_mode.rst
   normal_mode.rst


AtomSim Topics
***************
.. toctree::
   :maxdepth: 1


   cli_args.rst
   code_structure.rst
   adding_target.rst
