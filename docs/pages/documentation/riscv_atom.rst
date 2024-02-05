RISC-V Atom CPU
################
Atom is an open-source 32-bit soft-core processor written in Verilog. It is an embedded class processor 
architecture that implements the open-source RISC-V instruction set architecture (RV32IC), as described 
in the RISC-V unprivileged spec. Atom contains a two stage pipeline inspired from arm cortex m0+.

The following diagram showcases the architecture of RISC-V Atom core. 

.. image:: /diagrams/RISCVAtom_arch.png

Atom Pipeline Stages
*********************
The pipeline is divided into two stages. These are explained below.

Stage-1: Fetch
===============

Fetch unit is responsible for fetching instructions from instruction memory through the IPort. It uses a 32-bit Program
Counter (PC) to keep track of the address of the instruction being fetched. After the instruction is successfully fetched,
Program counter is incremented by either 4 or 2 (in case of compressed instruction). Fetch stage also includes pipeline
control logic which controls pipeline stalls and flushes. If compressed extension in enables, Fetch stage includes
RISC-V Compressed Aligner which aligns all the memory requests to 4 byte boundary. It also includes RISC-V Compressed
instruction decoder, which decodes 16-bit compressed instructions to their 32-bit equivalents.

Stage-2: Decode, Execute & Write-back
======================================

In this stage, the instruction from Instruction Register (IR) is decoded and executed. First, the decode unit decodes the
instruction and sets all the control signals in order to configure the data-path to execute the instruction. Parallelly,
operand registers are fetched and 32-bit immediate value is generated. Next, ALU/Comparator does the necessary
computation/comparison and the results are written back to register file. In case current instruction invokes a memory
request, stage-2 is stalled until response is received. Memload module is used extract the correct data from the received
memory response. If CSR extension is enabled, CSR Unit is included in this stage. CSR Unit provides Control and status
registers which perform various special functions. SR Unit also includes exceptions and interrupt handling logic if Exceptions
and interrupts are enabled. Branch calculation also happens in this stage and if branch is taken, a signal is sent to
the pipeline control logic to flush the pipeline.

Atom Interface
***************
Atom module is defined in the file ``RVATOM/rtl/core/atomRV.v``. It has has two independent ports (IPort & DPort)
which it uses to access memory. Both the ports use a generic ready-valid handshaking protocol to transfer data. We also
provide wrappers to the core to convert the generic handshaking protocol to standard bus protocols such as Wishbone.
These wrappers are specified in the following files.

#. Wishbone-B4 Wrapper with separate instruction and data port: ``RVATOM/rtl/core/atomRV_wb.v``


Atom Configuration operations
******************************

+----------------+------------------------------------------------------+
| Macro          | Function                                             |
+================+======================================================+
| ``EN_RVC``     | Enables support for RISC-V Compressed Extension      |
+----------------+------------------------------------------------------+
| ``EN_RVZICSR`` | Enables Control and Status Registers (CSRs)          |
+----------------+------------------------------------------------------+
| ``EN_EXCEPT``  | Enables support for RISC-V interrupts and exceptions |
+----------------+------------------------------------------------------+
| ``DPI_LOGGER`` | Enable DPI Logger                                    |
+----------------+------------------------------------------------------+


RISC-V Atom RTL
****************
RISC-V Atom is written in Verilog. The RTL specification for Atom is divided into 3 categories, *core*, *uncore* and *soc*,
all of which reside in the ``rtl`` directory.

Core Directory
==============
The ``rtl/core`` subdirectory contains the the *core* components of the CPU such as register file, ALU, decode unit etc.
It also contains Verilog header files like ``Defs.vh`` and ``Utils.vh``. ``Defs.vh`` contain various signal enumerations
and other parameters internal to the processor. ``Utils.vh`` defines some useful utility macros.


Uncore Directory
================
The ``rtl/uncore`` subdirectory contains the SoC peripheral components such as UART, GPIO, memories, etc. It also
contains the wishbone wrappers of some non-wishbone components. SoC implementations of the Atom processor usually
instantiate these hardware modules in their implementations.


SoC directory
==============
The top level Verilog modules for SoCs (such as AtomBones & HydrogenSoC) are present in the ``rtl/soc`` directory in the
corresponding subdirectories. Each of these top level modules are configured by their respective configuration headers
(<name>_Config.vh file). These configuration headers contain the macros used in the top-module definitions to control the
generation of SoC, various sub-components and their parameters.


RTL Features
*************

DPI Logger
===========
DPI Logger is a System Verilog DPI based logging mechanism provided with the RTL. It can be used to dump useful run-time
debug information such as PC values, Jump addresses, Loads and Stores, etc. into a log file. This module is present in
the ``rtl/dpi`` subdirectory.

To enable DPI Logger simply define ``DPI_LOGGER`` macro in the top-module or in the CLI as ``-DDPI_LOGGER``. This will
trigger the inclusion of the ``rtl/dpi/util_dpi.vh`` header in ``rtl/core/Utils.vh``. User is free include the
``rtl/core/Utils.vh`` header file in any Verilog file that needs to be debugged.

To log information, user first needs to call the ``dpi_trace_start()`` function somewhere in the rtl as following.

.. code-block:: verilog
  
  initial begin
    dpi_trace_start();
  end

Then the ``dpi_trace()`` function can be used to dump information. Its syntax is exactly same as the Verilog ``$display``
system function. Example of logging the jumps during code execution is provided in the ``AtomRV.v`` file and is also
shown below.

.. code-block:: verilog

    `ifdef DPI_LOGGER
        initial begin
            dpi_logger_start();     // begin logging
        end
    `endif

    `ifdef LOG_RVATOM_JUMP
    always @(posedge clk_i) begin
        if(jump_decision)   // on some trigger condition
            dpi_logger("Jump  address=0x%x\n", {alu_out[31:1], 1'b0});      // dump information
    end
    `endif

For logging the Jumps, user must also define the ``LOG_ATOMRV_JUMP`` macro in a similar way. This will generate a ``run.log``
file in the current directory containing all the dumped information.
