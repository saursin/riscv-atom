SoC Targets
############

SoC Targets are RISC-V Atom based SoCs. These contain one or more Atom cores, peripheral IPs, crossbars memories etc.
From a complexity and functionality point-of-view, SoC Targets can be as simple as a wrapper to the core interface
(e.g. AtomBones) and as complex as multi-core SOCs.

.. _soctarget-atombones:

AtomBones
**********
AtomBones is a stub-target that consists of a single atom core only. All other peripherals like memories and serial port
are simulated in C++. It is meant to be used for simulation and debugging of the core because of its faster simulation
rate. Unlike FPGA SoC targets, AtomBones provides larger memories for experimenting with the programs without being
constrained by physical memory size.

The following figure shows the architecture of AtomBones.

.. image:: /diagrams/AtomBones.png
   :width: 600

AtomBones backend implements the C++ simulation of SoC peripherals. In each cycle, the backend listens to IPort and DPort
for any requests and responds to them by bit-banging. The backend provides the ``fetch()`` and ``store()`` API that are
used to access these memory mapped peripherals.

.. _soctarget-hydrogensoc:

HydrogenSoC
************
HydrogenSoC is a full SoC implementation that contains a single Atom core along with Memories, and peripheral IPs like
UART, GPIOs, timers, etc. All the peripherals are connected to the CPU using a Wishbone-B4 bus. Users have the flexibility
to disable/enable core features, add/remove IPs, and set memory maps through a config JSON file. HydrogenSoC is proven on
multiple FPGA platforms.

The following diagram shows the architecture of HydrogenSoC.

.. image:: /diagrams/HydrogenSoC.png
   :width: 600
