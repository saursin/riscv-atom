Memory Map and Boot Flow
#########################

Memory Map
***********

The following image shows the memory map of Atom-based SoCs.

.. image:: /diagrams/memory_map.png

The column **C** shows the memory map template for all RISC-V Atom based SoCs. Column **D** and **E** show the memory map
for AtomBones and HydrogenSoC respectively.

Boot Flow
**********
Upon reset, the core jumps to the reset vector (default: ``0x10000``) which points to the start of BootROM. RISC-V Atom
BootROM includes a **stage-0 bootloader** that is automatically built and included when building the AtomSim or running FPGA
builds. The stage-0 bootloader uses RAM for stack. It initializes the platform, loads the user program in RAM and finally
transfers control to the user program. The user program often includes a **stage-1 bootloader** which sets up the runtime
environment and standard library before executing the application.

.. graphviz::

    digraph G {
       rankdir=LR;
       reset_vec [label="Reset Vector", shape=rect, style=filled, fillcolor=white];
       subgraph cluster_0 {
            style=filled;
            fillcolor=gray;
            label="BootROM";
            stage0_bl [label="Stage-0\nBootloader", shape=rect, style=filled, fillcolor=yellow];
        }
        subgraph cluster_1 {
            style=filled;
            fillcolor=gray;
            label="RAM";
            stage1_bl [label="Stage-1\nBootloader", shape=rect, style=filled, fillcolor=orange];
            prog [shape=rect, label="User Program", style=filled, fillcolor=cyan];
            stage1_bl -> prog [label="launch"];
        }
        reset_vec -> stage0_bl;
        stage0_bl -> stage1_bl;
    }

To know more about the stage-0 bootloader, see :doc:`this</pages/documentation/bootloader>` page.