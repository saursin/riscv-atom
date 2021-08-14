***********
Installing
***********

Clone the repository
=====================
::

  $ git clone https://github.com/saurabhsingh99100/riscv-atom.git
  $ cd riscv-atom

Adding to Path
================
RVATOM Environment Variable
----------------------------
`RVATOM` environment variable must point to root of riscv-atom directory for the tools to work properly.
For this purpose, append the following line in your `.bashrc` file.

::

  export RVATOM="your_path/riscv-atom" # export RVATOM variable

replace "`your_path`" in above command to your machine specific absolute path to the riscv-atom directory.

.. note:: All the commands are executed from the root directory unless explicitly mentioned. We'll refer 
  to this root directory as `RVATOM`.

Build/bin Directory
--------------------
For convenience, build/bin directory should also be added to path. This will allow you to invoke atomsim from anywhere.
For this purpose, append the following line in your `.bashrc` file.

::

  export PATH="$RVATOM/build/bin:$PATH"


Building the Simulator
=======================
Let's build AtomSim simulator for `atombones` target.

::
  
  $ make sim Target=atombones

This will create a build/bin directory inside the parent directory. The simulation executable will be available there.