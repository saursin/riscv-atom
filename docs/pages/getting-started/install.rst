***********
Installing
***********

Clone the repository
=====================
::

  $ git clone https://github.com/saurabhsingh99100/riscv-atom.git
  $ cd riscv-atom

Building the Simulator
=======================
Let's build AtomSim simulator for atombones.

::
  
  $ make sim Target=atombones

This will create a build/bin directory inside the parent directory. The simulation executable will be available here.

::

  $ cd build/bin 
  $ ./atomsim ../example/hello/hello.elf


Adding to Path
================
For convenience, build/bin directory can be added to path by adding the following line to your .bashrc file. This will allow you to invoke atomsim from anywhere.

::
  
  export PATH=<your_path>:$PATH

just replace your_path with the path to the riscv-atom/build/bin directory on your machine.