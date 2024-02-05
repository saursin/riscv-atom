SCAR: Search Compile Assert Run
################################

SCAR is a processor verification framework in python. SCAR performs a set of assembly level tests to verify the processor
implementation. Each assembly test checks for one particular functionality of the processor. SCAR does this by examining
a state dump file after the processor is done with executing a test code. This state dump file then checked assuming a
set of assertions in the form of expected register values. These assertions are provided in a separate assertion file.
SCAR is also used to verify the ISA-compliance.

SCAR is located in ``RVATOM/test/scar`` directory.

SCAR Workflow
**************
As the name suggests:

#. **Search:** SCAR searches for all the available assembly level tests specified in a config JSON file. The config JSON
   file also specifies the corresponding assertion files.
#. **Compile:** SCAR then compiles all the tests with a user-defined linker script.
#. **Execute:** In this step, The elf files are executed on the target simulator with a special flag which creates a
   state dump file after execution terminates. SCAR createsa ``work`` directory to store all the outputs.
#. **Verify:** Finally, Assertions are read from the assertion file and are then used to verify the register values in
   the generated state dump file.

Once you're finished building the RISC-V Atom project, SCAR can be invoked from the ``RVATOM`` directory itself using the
``scar`` target in the Makefile (see make help). Following is a demo output of a run of SCAR framework on hydrogensoc.

.. code-block:: bash

  +------------------------------------------------------------------------------+
  |                           SCAR Verification Report                           |
  +------------------------------------------------------------------------------+
  Date: 2024-02-04 19:23:21

  0).     add64                - Passed   All Assertions Passed
  1).     addi                 - Passed   All Assertions Passed
  2).     add                  - Passed   All Assertions Passed
  3).     andi                 - Passed   All Assertions Passed
  4).     and                  - Passed   All Assertions Passed
  5).     auipc                - Passed   All Assertions Passed
  6).     beq                  - Passed   All Assertions Passed
  7).     bge                  - Passed   All Assertions Passed
  8).     bgeu                 - Passed   All Assertions Passed
  9).     blt                  - Passed   All Assertions Passed
  10).    bltu                 - Passed   All Assertions Passed
  11).    bne                  - Passed   All Assertions Passed
  12).    function_call        - Passed   All Assertions Passed
  13).    jalr                 - Passed   All Assertions Passed
  14).    jal                  - Passed   All Assertions Passed
  15).    li                   - Passed   All Assertions Passed
  16).    load_store_byte      - Passed   All Assertions Passed
  17).    load_store_hw        - Passed   All Assertions Passed
  18).    lui                  - Passed   All Assertions Passed
  19).    lw                   - Passed   All Assertions Passed
  20).    mv                   - Passed   All Assertions Passed
  21).    ori                  - Passed   All Assertions Passed
  22).    or                   - Passed   All Assertions Passed
  23).    slli                 - Passed   All Assertions Passed
  24).    sll                  - Passed   All Assertions Passed
  25).    slti                 - Passed   All Assertions Passed
  26).    sltiu                - Passed   All Assertions Passed
  27).    slt                  - Passed   All Assertions Passed
  28).    sltu                 - Passed   All Assertions Passed
  29).    srai                 - Passed   All Assertions Passed
  30).    sra                  - Passed   All Assertions Passed
  31).    srli                 - Passed   All Assertions Passed
  32).    srl                  - Passed   All Assertions Passed
  33).    stack                - Passed   All Assertions Passed
  34).    storew               - Passed   All Assertions Passed
  35).    sub                  - Passed   All Assertions Passed
  36).    sw                   - Passed   All Assertions Passed
  37).    xori                 - Passed   All Assertions Passed
  38).    xor                  - Passed   All Assertions Passed
  ================================================================================
  Passed tests  : 39 / 39
  Ignored tests : 0 / 39
  Failed tests  : 0 / 39

How to Create Your Own Tests?
******************************
#. Assembly file containing the test source code must have a ``_start`` label before the start of code.
#. Assembly file must have a ``ebreak`` instraction after the end of code, this triggers the simulation to terminate.
#. Each test file must have a corresponding assertion file (``*.asrt``) both of which need to be specified in the JSON
   config file.
#. The assertion file uses a python like syntax to specify one expressions per line; which could use physical register
   names (x0, x1, x2...) or ABI names (zero, ra, sp) as variables.
#. State dump file generated by simulator must have one key value pair per line.
