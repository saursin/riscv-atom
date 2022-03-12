*****************
RISC-V Atom RTL
*****************
RISC-V Atom is written in Verilog. Its RTL is divided into 2 categories, core and uncore, 
both of which reside in the `rtl` directory in `core` and `uncore` subdirectories respectively.

Rtl directory
==============
The top level verilog modules (atombones & hydrogensoc) are present in the `rtl` directory. Each 
of these top level modules are configured by their respective configure headers (<name>_Config.vh file).
These configuration headers contain the macros used in the top-module definitions to control the 
generation of various sub-components and their parameters.


Core directory
---------------
`core` directory contains the the *core* components of the CPU such as register file, alu, 
decode unit etc. It also contains verilog header files like `Defs.vh` and `Utils.vh`. `Defs.vh`
defines various signal eunumerations and other parameters internal to the processor. `Utils.vh` 
defines some useful utility macros.


Uncore directory
-----------------
The `uncore` subdirectory contains all the peripheral components such as uart, gpio, ram, rom etc. 
This also includes the wishbone wrappers of some non-wishbone components. SoC implementations of the 
Atom processor usually instantiate these hardware modules in their implementations.

RTL Features
=============

DPI Logger
-----------
DPI Logger is a SystemVerilog DPI based logging mechanism provided with the RTL. It can be used to 
dump useful run-time debug information such as PC values, Jump addresses, Loads and Stores, etc.
into a log file. This module is present in the `rtl/dpi` subdirectory.

To enable DPI Logger simply define `DPI_LOGGER` macro in the topmodule or in the Makefile VFLAGS variable
as `-DDPI_LOGGER`. This will trigger the inclusion of the `rtl/dpi/util_dpi.vh` header in `rtl/core/Utils.vh`. 
User is free include the `rtl/core/Utils.vh` header file in any verilog file that needs to be debugged.

To log information, user first needs to call the `dpi_trace_start()` function somewhere in the rtl as following.

.. code-block:: verilog
  
  initial begin
    dpi_trace_start();
  end

Then the `dpi_trace()` function can be used to dump information. Its syntax is exactly same as the verilog 
`$display` system funciton. Example of logging the jump is provided in the `AtomRV.v` file and is also shown 
below.

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

For logging the Jumps, user must also define the `LOG_ATOMRV_JUMP` macro in a similar way. This will generate a 
"run.log" file in the current directory containt all the dumped information.


