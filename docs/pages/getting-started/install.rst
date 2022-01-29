***********
Installing
***********

Clone the repository
=====================
::

  $ git clone https://github.com/saurabhsingh99100/riscv-atom.git
  $ cd riscv-atom

Adding RISC-V atom environment variables to $PATH
==================================================

RVATOM
-------
`RVATOM` environment variable must point to root of riscv-atom directory for the tools & scripts to work properly.

RVATOM_LIB
-----------
`RVATOM` environment variable must point to the `riscv-atom/sw/lib` folder. This variable is used by the compile 
scripts to locate libc for risc-v atom.

We must define `RVATOM` and `RVATOM_LIB` envronment variables in our `.bashrc file`. For convenience, `RVATOM/build/bin` 
directory should also be added to path. This will allow you to invoke atomsim and other scripts and tools from anywhere.

For this purpose, append the following lines in your `.bashrc` file.

::

  # export RVATOM variable
  export RVATOM="<your-path>/riscv-atom"

  # export RVATOM_LIBS variable
  export RVATOM_LIB="$RVATOM/sw/lib"

  # add $RVATOM/build/bin to path
  export PATH="$RVATOM/build/bin:$PATH"

replace "`your_path`" in above command to your machine specific absolute path to the riscv-atom directory.

.. note:: All the commands are executed from the root directory unless explicitly mentioned. We'll refer 
  to this root directory as `RVATOM`.


Building the Simulator
=======================
Let's build AtomSim simulator for `atombones` target.

::
  
  $ make Target=atombones

This will create a build/bin directory inside the parent directory. The simulation executable will be available there.