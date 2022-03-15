SCAR: Search Compile Assert Run
################################

SCAR is a processor verification framework in python. SCAR performs a set of assembly level tests to verify the processor implementation. Each assembly test checks for one particular functionality of the processor. SCAR does this by examining a state dump file after the processor is done with executing a test code. This state dump file then checked assuming a set of assertions in the form of expected register values. These assertions are provided in the assembly file itself. SCAR is also used to verify the ISA-compliance.

SCAR Workflow
**************

#. **Search:** SCAR searches for all the available assembly level test in the directory and makes a list 
#. **Compile:** It then compiles all the found tests with a user-defined linker script.
#. **Execute:** In this step, The elf files are executed on the target simulator which creates a state dump file after execution.
#. **Verify:** Finally, Assertions are read from the assembly file containing the test. These are then used to check for mismatches in the generated state dump file.


Assembly test format
*********************
The assembly file must satisfy the following criteria:

#. File must have a ``_start`` label before the start of code.
#. File must have a ``ebreak`` instraction after the end of code.
#. File must have an assertion section at the bottom with the following format.

Assertion Section Format
========================
The assembly file must contain a set of assertions at the bottom in the following format:

::

    .global _start
   _start:
   
   li t0, 0x00d01010
   li t1, 0x1ddc1044
   li t2, 0xdeadbeef
   li t3, 0x22101301
   li t4, 0xfaf01569
   li t5, 0x078b102a
   li t6, 0xdae013c0
   
   add a0, t0, t1
   add a1, t1, t2
   add a2, t2, t3
   add a3, t3, t4
   add a4, t4, t5
   add a5, t5, t6
   
   nop
   nop
   ebreak
   
   # $-ASSERTIONS-$
   # eq a0 0x1eac2054
   # eq a1 0xfc89cf33
   # eq a2 0x00bdd1f0
   # eq a3 0x1d00286a
   # eq a4 0x027b2593 
   # eq a5 0xe26b23ea


State-Dump file format
=======================
[TODO]