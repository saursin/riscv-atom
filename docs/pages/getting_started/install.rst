Installation
#############

Clone the repository
*********************
::

  $ git clone https://github.com/saursin/riscv-atom.git
  $ cd riscv-atom     # switch to riscv-atom directory


.. note:: All the commands are executed from the root directory unless explicitly mentioned. We'll refer to this root directory as ``RVATOM_ROOT``.


Add RISC-V Atom environment variables to $PATH
**************************************************

#. ``RVATOM`` environment variable must point to root of riscv-atom directory for the tools & scripts to work properly.
#. ``RVATOM`` environment variable must point to the ``RVATOM_ROOT/sw/lib`` folder. This variable is used by the compile scripts to locate *libcatom*.

Define ``RVATOM`` and ``RVATOM_LIB`` envronment variables in our ``.bashrc`` file. For convenience, ``RVATOM_ROOT/build/bin`` directory should also be added to path. This will allow you to invoke atomsim and other scripts and tools from anywhere.

For this purpose, append the following lines in your ``.bashrc`` file.

.. code-block:: bash

  # export RVATOM variable
  export RVATOM="<path_to_RVATOM_ROOT>"

  # export RVATOM_LIBS variable
  export RVATOM_LIB="$RVATOM/sw/lib"

  # add $RVATOM/build/bin to path
  export PATH="$RVATOM/build/bin:$PATH"


replace "`path_to_RVATOM_ROOT`" in above command to your machine specific absolute path to the riscv-atom directory.

If you are hesitant in modifying the ``.bashrc`` file, you may alternatively source the provided ``sourceme`` file every time you work with the project.

.. code-block:: bash

  source sourceme


Building the Simulator
***********************
Let's build AtomSim simulator for ``atombones`` target.

::
  
  $ make Target=atombones

This will create a build/bin directory inside the parent directory. The simulation executable will be available there.
Assuming your ``PATH`` is set properly, try the following command to verify installation.

.. code-block:: bash
  
  $ atomsim --help
  AtomSim v2.0
  RTL simulator for Atom based systems [ hydrogensoc ]
  Usage:
    atomsim [OPTION...] input
  ...