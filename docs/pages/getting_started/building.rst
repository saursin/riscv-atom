Building RISC-V Atom
######################

Clone the repository
*********************
First let's clone the repository as follows.

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

For convenience, ``RVATOM/sourceme`` script is provided that you can source as follows:

.. code-block:: bash

  $ source sourceme

.. tip::
  With this method, every time you open a new terminal, you have to source the ``sourceme`` file. You can optionally
  append the aforementioned to your ``.bashrc`` to source it automatically every time you open a new terminal.

  .. code-block:: bash

    $ echo "source <rvatom-path>/sourceme" >> ~/.bashrc
  
  Replace ``rvatom-path`` with the path to your RISC-V atom directory.


Selecting a target
*******************
Every build is for one SoC target (``soctarget``), e.g. :ref:`AtomBones <soctarget-atombones>` or ``hydrogensoc``.
There is no default; set one once with ``make config``, which saves it in ``RVATOM/config.mk`` (not tracked by git):

.. code-block:: bash

  $ make config soctarget=atombones     # set the default soctarget
  $ make config                         # show the current soctarget and where it comes from

A ``soctarget`` given on the command line (``make sim soctarget=hydrogensoc``) or exported in the environment
(``export soctarget=hydrogensoc``) takes precedence over ``config.mk``.


Building AtomSim
*****************
:doc:`AtomSim</pages/documentation/atomsim/atomsim>` is the interactive RTL simulator for RISC-V Atom. Build it,
along with *libcatom* and the bootloader, from the root directory:

.. code-block:: bash

  $ make                    # same as 'make default': lib + bootloader + atomsim

Optionally, to speed up builds, you can specify the number of parallel jobs to run using ``-j <njobs>`` flag in the
above command. All build output goes to ``RVATOM/build/<soctarget>/``; the source tree is never written to (except
example programs, which build in ``sw/examples/<example>/``):

.. code-block:: text

  build/<soctarget>/
  ├── vars.<soctarget>.mk       # build parameters derived from rtl/config/<soctarget>.yml
  ├── rtl/                      # filelist + staged RTL sources
  ├── sim/bin/atomsim           # the simulator
  ├── sw/lib/libcatom.a
  └── sw/bootloader/bootloader.hex

Each target has its own build directory, so switching targets never requires a clean. To build somewhere else, set
``RVATOM_BUILD=<path>``; use a separate path per target, since a custom build directory is not split by target.

To run ``atomsim`` directly (as the rest of this documentation does), add the target's build to your ``PATH``:

.. code-block:: bash

  $ export PATH=$RVATOM/build/atombones/sim/bin:$PATH
  $ atomsim --version
  v2.2 [ atombones ]
  ...

Make targets such as ``make test``, ``make scar`` and ``run`` in ``sw/examples`` always use the selected target's
atomsim, regardless of ``PATH``.


How the build works
********************
Each component (``rtl``, ``sim``, ``sw/lib``, ``sw/bootloader``, ``sw/examples``, ``synth/*``) has its own Makefile,
which you can also invoke directly, e.g. ``make -C sw/examples ex=banner compile run``. All of them include
``RVATOM/common.mk``, which:

#. resolves ``soctarget`` (command line, environment, then ``config.mk``) and sets
   ``RVATOM_BUILD ?= RVATOM/build/<soctarget>``;
#. generates ``vars.<soctarget>.mk`` in the build directory by running
   ``scripts/cfgparse.py -t make-vars`` on the target's YAML config, and includes it. It defines ``RVATOM_ISA``,
   ``RVATOM_ABI``, ``RVATOM_TOPMODULE``, ``RVATOM_CDEFINES`` and ``RVATOM_VDEFINES``.

The generated file is regenerated whenever the target's YAML, any YAML it imports, or ``cfgparse.py`` changes, and
the library, bootloader, RTL filelist, simulator and examples depend on it. Editing a config therefore rebuilds
exactly what it affects; no ``make clean`` is needed. ``make show-config`` prints the resolved values.

Useful targets (run ``make help`` for the full list):

.. code-block:: bash

  $ make test               # run the banner example on atomsim
  $ make lint               # Verilator lint of the target's RTL
  $ make scar               # SCAR instruction tests (pytest)
  $ make clean              # clean the current soctarget's build
  $ make clean-all          # remove build/ for all targets

FPGA flows under ``synth/`` default to ``hydrogensoc`` and use their own ``build_dir`` as ``RVATOM_BUILD``, so they
never touch the simulation build.
