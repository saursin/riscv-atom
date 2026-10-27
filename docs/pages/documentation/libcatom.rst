Libcatom: C standard library for RISC-V Atom
#############################################

Libcatom is a minimal C standard library for RISCV-Atom. It consists of startup code, basic stdio library, heap allocator,
soc-specific libraries such as drivers for peripherals such as GPIO, UART, SPI etc. and linker scripts. The soc-specific 
sources (such as drivers) are located inside the corresponding folder. ``platform.h`` header defines most of the 
platform-specific macros for libcatom. Libcatom does not define the software multiply and soft floating point operations 
and therefore uses the standard library provided with the compiler.

Building Libcatom
******************
Libcatom can be built as follows.


.. code-block:: bash

  $ make soctarget=hydrogensoc lib


.. tip::
    You don't need to build Libcatom separately in most cases as it will be built automatically (with correct soc target)
    while building AtomSim.

.. note::
    The same library and programs run in AtomSim and on hardware. AtomSim decodes UART output at the baud rate set in the
    UART divisor register.