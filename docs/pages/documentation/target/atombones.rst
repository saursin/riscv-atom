**********
AtomBones
**********
AtomBones is a stub target for AtomSim. It consists of only the RISCV-Atom core. whenever the project 
is built with a `Target=atombones` flag it performs the following actions:

1. compiles the `Atomsim` program with the `AtomBones Backend`.
2. Verilates the required Verilog files, and
3. links everything to build the simulator.

AtomBones Backend
=================
[TODO]