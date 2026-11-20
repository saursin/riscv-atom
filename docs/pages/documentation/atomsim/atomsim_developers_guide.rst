AtomSim Developer's Guide
##########################

AtomSim Code Structure
=======================

AtomSim is designed in a modular fashion to allow portability across platforms not just limited to RISC-V Atom. Checkout
:ref:`AtomSim architecture<atomsim_arch>`. The source code for atomsim is located under ``RVATOM/sim`` folder. Atomsim
comprises of an interactive frontend (implemented in ``interactive.cpp``), middle-end (implemented in ``simstate.cpp``)
and a backends for different *soctargets* (implemened in ``backend_<soctarget>.cpp``). All AtomSim backends are
implemented as a child-class of the *Backend* template class (implemented in ``backend.hpp``). The backend class
implements the RTL facing side of AtomSim and is responsible for reading register values and driving any signals through
the *Testbench* class (implemented in ``testbench.hpp``).

Adding a New Backend to AtomSim
================================

- Adding new backend involves adding a new backend class that extends the *Backend* class (in ``backend.hpp``).
- All soctarget backends must override all the methods marked with ``[** OVERRIDE **]`` comment in the *Backend* class
  (parent) as bare-minimum.
- The methods marked with ``[** MAY OVERRIDE **]`` can optionally be overriden in child classes depending on the intended
  functionality.
- To allow AtomSim to *peek* and *poke* into SoC memory, user must override the ``fetch()`` and ``store()`` methods.
- The backend may optioanlly use the testbench class from (``testbench.hpp``) to implement the low-level RTL facing 
  functionality.
- Models of things wired to the SoC's pins are *Device* subclasses (``device.hpp``): the backend keeps a list of them
  and calls ``eval()`` on each once per cycle. *BitbangUART* (``bitbang_uart.cpp``) is a terminal on the UART pins that
  relays to a *Vuart* (``vuart.cpp``: a pseudo-terminal or a Linux serial port) and/or stdout; *SPIDevice*
  (``spi_device.cpp``) is the base for SPI models such as *SPIFlash* (``spi_flash.cpp``).
- Users may intend to not simulate the whole SoC in RTL and therefore they can create their own C++ modules to emulate 
  some functionality and call them from their backends. One example of this is the :ref:`AtomBones<soctarget-atombones>`
  soctarget which only simulates the processor in RTL, everything else like memories (implemented in ``memory.cpp``),
  uart, etc. is emulated in C++.