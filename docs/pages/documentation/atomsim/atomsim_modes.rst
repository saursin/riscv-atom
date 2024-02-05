AtomSim Simulation Modes
#########################

.. _atomsim_normal_mode:

AtomSim can be run in 2 modes, normal and interactive mode.

Normal Mode
************
In this mode of simulation, no debug information is printed. Optionally user can enable printing of UART traffic to
stdout using ``--enable-uart-dump / -u`` CLI option while invoking atomsim. Using ``--verbose / -v`` flag shows
additional useful information. Checkout :doc:`cli_args` for information on CLI options.

.. code-block:: bash
  
  $ atomsim sw/examples/banner/banner.elf -v
     ___  __             _____
    / _ |/ /____  __ _  / __(_)_ _
   / __ / __/ _ \/  ' \_\ \/ /  ' \
  /_/ |_\__/\___/_/_/_/___/_/_/_/_/  v2.2
  soctarget: hydrogensoc
  Initializing ram
  Initialization complete!
  EBreak hit at 0x2000007c
  Exiting..


.. _atomsim_debug_mode:

Debug/Interactive Mode
***********************
Debug/Interactive mode of AtomSim presents a console interface to the user, allowing them to interact with the simulation
in real-time. The console is similar to the GDB console and inherits many features from it like stepping, breakpoints etc.

To invoke AtomSim in interactive debug mode, invoke atomsim with ``--debug / -d`` flag.

.. code-block:: bash

     ___  __             _____
    / _ |/ /____  __ _  / __(_)_ _
   / __ / __/ _ \/  ' \_\ \/ /  ' \
  /_/ |_\__/\___/_/_/_/___/_/_/_/_/  v2.2
  [         1] PC: 0x00000000, IR: 0x00000013, _
  atomsim>


As shown above, AtomSim will display the current cycle count, PC value, Instruction Register value and its disassembly
respectively. To see register file contents, users can use the ``info / i`` command in the AtomSim console.

Alternatively, If invoked with both ``--debug / -d`` and ``--verbose / -v`` CLI options, AtomSim presents a more verbose
interface with register file contents in each cycle.

.. code-block:: bash

     ___  __             ____
    / _ |/ /____  __ _  / __(_)_ _
   / __ / __/ _ \/  ' \_\ \/ /  ' \
  /_/ |_\__/\___/_/_/_/___/_/_/_/_/  v2.2
  soctarget: hydrogensoc
  Initializing ram
  Initialization complete!
  ┌─[         1]─────────────────────────────────────────────┐
  │ PC: 0x00000000    PC_f: 0x00010000     (    +65536   )   │
  │ IR: 0x00000013    _                                      │
  └──────────────────────────────────────────────────────────┘
    x0  (zero) : 0x00000000    x16 (a6)   : 0x00000000
    x1  (ra)   : 0x00000000    x17 (a7)   : 0x00000000
    x2  (sp)   : 0x00000000    x18 (s2)   : 0x00000000
    x3  (gp)   : 0x00000000    x19 (s3)   : 0x00000000
    x4  (tp)   : 0x00000000    x20 (s4)   : 0x00000000
    x5  (t0)   : 0x00000000    x21 (s5)   : 0x00000000
    x6  (t1)   : 0x00000000    x22 (s6)   : 0x00000000
    x7  (t2)   : 0x00000000    x23 (s7)   : 0x00000000
    x8  (s0/fp): 0x00000000    x24 (s8)   : 0x00000000
    x9  (s1)   : 0x00000000    x25 (s9)   : 0x00000000
    x10 (a0)   : 0x00000000    x26 (s10)  : 0x00000000
    x11 (a1)   : 0x00000000    x27 (s11)  : 0x00000000
    x12 (a2)   : 0x00000000    x28 (t3)   : 0x00000000
    x13 (a3)   : 0x00000000    x29 (t4)   : 0x00000000
    x14 (a4)   : 0x00000000    x30 (t5)   : 0x00000000
    x15 (a5)   : 0x00000000    x31 (t6)   : 0x00000000
  atomsim>

Try the ``help`` command to checkout the commands available in AtomSim console.

Tips for using AtomSim in interactive mode
===========================================
- If simulation is run in normal mode, pressing :kbd:`ctrl` + :kbd:`c` returns AtomSim to interactive mode and pressing
  :kbd:`ctrl` + :kbd:`c` in interactive mode terminates the simulation.
- AtomSim has shorter aliases for most of the command names which can be seen in command help.
- AtomSim console is based on the linux readline library which allows user to press :kbd:`up` / :kbd:`down` arrow keys
  to cycle between previously entered commands.
- If user presses :kbd:`enter` without entering a command, AtomSim executes the last executed command.
- When entering numeric values in console such as addresses, users can specify in decimal, hexadecimal (by prefixing
  the value with **0x**), or binary (by prefixng the value with **0b**).
- Register names can be specified as physical register names (*x0, x1, x2 ...*) or their ABI names (*zero, ra, sp...*)
- Some of effects of CLI arguments can be overridden in the AtomSim console, like enabling/disabling trace, verbosity etc.
- Lastly, refer to the ``help`` command to find most up-to-date information related to the AtomSim console.
