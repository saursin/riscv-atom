AtomSim: A simulation tool for Atom based SoCs
###############################################

AtomSim is an interactive RTL simulator for Atom based SoCs. It provides an interface which is similar to the RISC-V 
Spike simulator, but simulates the actual RTL in the backend. Atomsim is a modular and flexible simulaion solution 
based on Verilator, due to which it can achieve a very high simulation rate. AtomSim is a feature rich tool which makes 
it very powerful for debugging code on the Atom CPU.

Key Features of AtomSim are listed below:

#. Achieves a high simulation rate due to use of Verilator.
#. Target Configurable, can be easily extended for new SoC designs.
#. In-built debug mode similar to spike.
#. External Debug Support using OpenOCD & GDB ``[TODO]``.
#. Supports VCD trace generation.
#. Supports memory dumps.
#. Compatible with RISC-V compliance tests framework.
#. Compatible with SCAR framework.

The following figure depicts the architecture of atomsim.

.. image:: ../../../diagrams/atomsim_arch.png

See :doc:`../../getting_started/building` for info on how to build atomsim.

To view available command line options, use:

.. code-block:: bash

  $ atomsim --help


Modes of Operation
*******************
Atomsim supports two modes of operation:

#. :ref:`Normal Mode <atomsim_normal_mode>`
#. :ref:`Debug Mode <atomsim_debug_mode>`

AtomSim Topics
***************
.. toctree::
   :maxdepth: 1

   cli_args.rst
   atomsim_modes.rst
   code_structure.rst
   adding_target.rst
