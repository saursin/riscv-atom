Building RISC-V Atom
######################

Clone the repository
*********************

.. code-block:: bash

  $ git clone https://github.com/saursin/riscv-atom.git
  $ cd riscv-atom     # switch to riscv-atom directory


.. note::
  All the commands are executed from the root directory unless explicitly mentioned. We'll refer to this root 
  directory as ``RVATOM``.


Setting up the environment
***************************

#. ``RVATOM`` environment variable must point to root of riscv-atom directory for the tools & scripts to work properly.
#. ``RVATOM_LIB`` environment variable must point to the ``RVATOM/sw/lib`` folder. This variable is used by the compile 
   scripts to locate *libcatom*.

For convenience, ``RVATOM/sourceme`` script is provided that you can source everytime you work with the project.
This can be done as follows:

.. code-block:: bash

  $ source sourceme

With this method, everytime you open a new terminal, you have to source the ``sourceme`` file. You can optionally append 
the aforementioned to your ``.bashrc`` to source it automatically everytime you open a new terminl.

.. code-block:: bash

  $ echo "source <rvatom-path>/sourceme" >> ~/.bashrc

In the above command replace ``rvatom-path`` with the path to your RISC-V atom directory.


Building AtomSim
*****************
AtomSim is the interactive RTL simulator for RISC-V Atom. Let's build AtomSim simulator for 
`AtomBones <../documentation/soc_targets.html#atombones>`_ target.

.. code-block:: bash
  
  $ make soctarget=atombones

Optionally, you can specify the number of parallel jobs to run using ``-j <njobs>`` flag in the above command.
This will create ``RVATOM/sim/build`` directory for AtomSim build files. You can find the Atomsim executable in 
``RVATOM/sim/build/bin`` directory. 

Assuming you've sourced the ``RVATOM/sourceme`` file, try the following command to check if the build was successful.

.. code-block:: bash
  
  $ atomsim --version
  v2.2 [ atombones ]
  ...