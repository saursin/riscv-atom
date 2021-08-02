******************************
SCAR: Search Compile and Run
******************************
SCAR is a processor verification framework in python. SCAR performs a set of assembly level tests to 
verify the processor implementation. Each assembly test checks for one perticular functionality of 
the processor. SCAR does this by examining a state dump file after the processor is done with executing 
a test code. This state dump file then checked assuming a set of assertions in the form of expected 
register values. These assertions are provided in the assembly file itself. SCAR is also used to 
verify the ISA-compliance.

SCAR Workflow
==============

1. **Search:** SCAR searches for all the available assembly level test in the directory and makes a 
   list 
2. **Compile:** It then compiles all the found tests with a user-defined linker script.
3. **Execute:** In this step, The elf files are executed on the target simulator which creates a 
   state dump file afte execution.
4. **Verify:** Finally, Assertions are read from the assembly file containing the test. These are 
   then used to check for mismatches in the generated state dump file.


Assembly test format
=====================
[TODO]

State-Dump file format
=======================
[TODO]