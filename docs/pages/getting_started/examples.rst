Running Examples on AtomSim
############################

The RISC-V Atom project consists of a wide range of examples programs out-of-the-box to test. These examples programs 
reside in `RVATOM/sw/examples` directory.

Switch to examples directory

.. code-block:: bash

  $ cd sw/examples

Lets run the classical "hello World!" example first!

Hello World Example 
********************
The source code for the *hello-world* example resides in the ``hello-asm`` directory. You can have a look at the source 
code. First we need to compile the hello world example with our RISC-V gcc cross-compiler. For this purpose, use the 
provided makefile as following.

.. code-block:: bash

  $ make soctarget=atombones ex=hello-asm compile

The above command should generate a ``hello.elf`` file in the ``hello-asm`` directory.

.. tip::
  ``soctarget=atombones`` in the above command can be skipped if AtomSim is already built. The SoC target will be automatically detected from 
  the AtomSim executable.

Now fire up AtomSim and provide the generated elf file as argument.

.. code-block:: bash

  $ atomsim -u hello-asm/hello.elf
     ___  __             _____
    / _ |/ /____  __ _  / __(_)_ _
   / __ / __/ _ \/  ' \_\ \/ /  ' \
  /_/ |_\__/\___/_/_/_/___/_/_/_/_/  v2.2
  ----------8<-----------8<-----------8<-----------8<---------

  **** RISC-V Atom Bootloader ****
  bootmode: 0x1
  Jumping to RAM
  --------------------------------
  Hello World!
        -- from Assembly

  Exit hit at 0x2000014a
  Exiting..

You should see *Hello world* message on the screen.

.. note::
  Make sure to use ``-u`` flag to direct UART output from SoC to stdout. You can also use ``-t`` flag to generate a VCD
  trace of the simulation.

Alternatively, use ``make run`` to run the example as follows

.. code-block:: bash

  $ make soctarget=atombones ex=hello-asm run


We can compile other examples also in the similar fashion by using the following syntax:

.. code-block:: bash

  $ make soctarget=<TARGET> ex=<EXAMPLE> compile
  $ make soctarget=<TARGET> ex=<EXAMPLE> run

.. note::
  Run ``$ make help`` to get more information about supported targets and examples.

Banner Example 
***************

.. code-block:: bash

  $ make target=atombones ex=banner compile

.. code-block::

  $ atomsim -u banner/banner.elf
     ___  __             _____
    / _ |/ /____  __ _  / __(_)_ _
   / __ / __/ _ \/  ' \_\ \/ /  ' \
  /_/ |_\__/\___/_/_/_/___/_/_/_/_/  v2.2
  ----------8<-----------8<-----------8<-----------8<---------

  **** RISC-V Atom Bootloader ****
  bootmode: 0x1
  Jumping to RAM
  --------------------------------
  
                    .';,.           ....;;;.
                   .ll,:o,                ':c,.
                   .dd;co'                  .cl,
                  .:o:;,.                     'o:
                  co.                          .oc
                 ,o'          .coddoc.          'd,
                 lc         .lXMMMMMMXl.         ll
                .o:         ;KMMMMMMMMK,         :o.
                .o:         'OMMMMMMMMO.         :o.
                 co.         .o0XNNX0o.         .oc
                 .o:           ..''..           :o.
                  'o:                          :o'
                   .lc.                      .ll.
                     ,lc'                  'cl,
                       'cc:,..        ..,:c:'
                          .;::::;;;;::::;.
                                ....
       ____  _________ _______    __         __
      / __ \/  _/ ___// ____/ |  / /  ____ _/ /_____  ____ ___
     / /_/ // / \__ \/ /    | | / /  / __ `/ __/ __ \/ __ `__ \
    / _, _// / ___/ / /___  | |/ /  / /_/ / /_/ /_/ / / / / / /
   /_/ |_/___//____/\____/  |___/   \__,_/\__/\____/_/ /_/ /_/
  /=========By: Saurabh Singh (saurabh.s99100@gmail.com)====/
  
  CPU      : RISC-V Atom @ 50000000 Hz
  Arch     : RV32IC - little endian
  CODE RAM : 0x20000000 (40960 bytes)
  DATA RAM : 0x2000a000 (8192 bytes)
  Exiting...
  Exit hit at 0x2000007c
  Exiting..


How to compile and run all examples?
*************************************
Instead of testing all examples one-by-one, we can compile and run all examples as follows.

.. code-block:: bash

  $ make soctarget=atombones run-all

Using Atomsim Vuart
********************
When using ``-u`` flag, AtomSim relays the output of the running application on stdout. But, in this mode of operation,
user cannot provide any input to the running program. AtomSim provides Virtual UART to work around this problem: it
connects the simulated UART to a serial port, so ``atomterm`` talks to the simulation exactly as it talks to an FPGA.


.. graphviz::

  digraph G {
    rankdir=LR;
    subgraph cluster_1 {
        style=filled;
        fillcolor=cyan;
        label="AtomSim";
        VUART [shape=rect, style=filled, fillcolor=gray];
        RTL [shape=rect, label="RTL\nSimulation", style=filled, fillcolor=gray];
        VUART -> RTL [dir=both, label="UART traffic"];
    }
    Atomterm [shape=rect, style=filled, fillcolor=cyan];
    Simtty [shape=rect, label="sim.tty\n(pseudo-terminal)", style=filled, fillcolor=orange];
    ttyUSB [shape=rect, style=filled, fillcolor=orange];
    FPGA [shape=box3d, style=filled, fillcolor=green];
    Atomterm -> Simtty -> VUART [dir=both];
    Atomterm -> ttyUSB [dir=both];
    ttyUSB -> FPGA [label=USB];
  }

|

Run an example with ``vuart=pty`` (atomsim's ``-p``): atomsim creates a pseudo-terminal (a virtual serial port),
links it as ``$RVATOM/sim.tty`` and waits for a terminal. In another terminal, run ``atomterm`` without a port:

.. code-block:: bash

  $ make -C sw/examples ex=hello-c run vuart=pty
  VUART on /home/user/riscv-atom/sim.tty (/dev/pts/6)
  waiting for a terminal: run atomterm

  $ atomterm
  [atomterm] connected to the simulator (/home/user/riscv-atom/sim.tty) at 115200 baud

The program starts once ``atomterm`` connects, and when it exits you are at the bootloader console, as on a board.
``--no-wait`` starts without waiting, and ``vuart=<port>`` (atomsim's ``--vuart-port=<port>``) uses an existing serial
port instead.


Adding New Examples
====================

To add a new example to the existing framework, simply create a directory under the ``RVATOM/sw/examples`` directory.

.. code-block:: bash

  $ mkdir myexample


Next, put your source files under this directory. 

.. code-block:: c

  $ cat myexample.c
  #include <stdio.h>
  void main()
  {
      char hello[] = "New Example\n";
      printf(hello);
      return;
  }


Finally add a new file named ``Makefile.include`` in the same directory which defines the name of the source files and 
executable file as follows.  


.. code-block:: bash

  $ cat Makefile.include
  src_files = myexample.c
  executable = myexample.elf


That's it! Now you can use the same compile and run commands as discussed earlier to run this example.

