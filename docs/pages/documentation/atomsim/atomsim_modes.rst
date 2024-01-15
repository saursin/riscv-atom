AtomSim Simulation Modes
#########################

.. _atomsim_normal_mode:

Normal Mode
************
In this mode of simulation, no debug information is printed. Only serial data recieved from the soc is printed to the 
stdout. Using ``--verbose / -v`` flag shows additional useful information.

.. code-block:: bash
  
  $ atomsim sw/examples/banner/banner.elf -v
  Input File: hello-asm/hello.elf
  Resetting..
  Relaying uart-rx to stdout (Note: This mode does not support uart-tx)
  Initialization complete!
  Hello World!
        -- from Assembly
  
  Haulting @ tick 931


.. _atomsim_debug_mode:

Debug/Interactive Mode
***********************
In this mode of simulation, Contents of Program counter (in both stages), Instruction register, instruction disassembly 
and contents of registers (if verbosity is set) are printed to stdout. A console with symbol ``:`` is also displayed at 
the bottom if screen for user to enter various commands to control the simulation. To step through one clock cycle, user 
can simply press :kbd:`enter` key (without entering anything in console).

To invoke interactive debug mode, invoke atomsim with `-d` & `-v` flag:

::

  $ ./build/bin/atomsim hello.elf -d -v
  Segments found : 2
  Loading Segment 0 @ 0x00000000 --- done
  Loading Segment 1 @ 0x00010000 --- done
  Entry point : 0x00000000
  Initialization complete!
  : 
  -< 1 >--------------------------------------------
  F-STAGE  |  pc : 0x00000034  (+4) () 
  E-STAGE  V  pc : 0x00000000   ir : 0x00010517   [addi x1, 0x33f]
  ---------------------------------------------------
   x0  (zero) : 0x00000000   x16 (a6)   : 0x00000000  
   x1  (ra)   : 0x00000000   x17 (a7)   : 0x00000000  
   x2  (sp)   : 0x00000000   x18 (s2)   : 0x00000000  
   x3  (gp)   : 0x00000000   x19 (s3)   : 0x00000000  
   x4  (tp)   : 0x00000000   x20 (s4)   : 0x00000000  
   x5  (t0)   : 0x00000000   x21 (s5)   : 0x00000000  
   x6  (t1)   : 0x00033000   x22 (s6)   : 0x00000400  
   x7  (t2)   : 0x00000000   x23 (s7)   : 0x00000000  
   x8  (s0/fp): 0x00000000   x24 (s8)   : 0x00000000  
   x9  (s1)   : 0x00000000   x25 (s9)   : 0x00000000  
   x10 (a0)   : 0x00000000   x26 (s10)  : 0x00000000  
   x11 (a1)   : 0x00000000   x27 (s11)  : 0x00000000  
   x12 (a2)   : 0x00000000   x28 (t3)   : 0x00000000  
   x13 (a3)   : 0x00000000   x29 (t4)   : 0x00000000  
   x14 (a4)   : 0x00000000   x30 (t5)   : 0x00000000  
   x15 (a5)   : 0x00000000   x31 (t6)   : 0x00000000  
  : 


Interacting With Debug Console
===============================

Displaying contents of a register
----------------------------------
Contents of register can be displayed simply typing its name (abi names are also supported) on the 
console. ex:

::

  : reg x0
  x0 = 0x000045cf
  : reg ra
  ra = 0x0000301e

Use ':' to display a range of registers. ex:

:: 

  : x0 : x1


Displaying Contents of a memory location
-----------------------------------------
::

  : m <address> <sizetag>

Address can be specified in hex or decimal.
Use sizetag to specify the size of data to be fetched, b for byte, h for half-word and w for word 
(default is word).

::

  : m 0x30 b
  mem[0x30] = 01

Use ':' to display contents of memory in a range. ex:

::
  
  : m 0x32:0x38 w
  mem[0x30] = 01 30 cf 21
  mem[0x38] = 11 70 ab cf


Generating VCD traces
----------------------
Tracing can be enabled by:

::

  : trace out.vcd
  Trace enabled : "./out.vcd" opened for output.

or by passing --trace <file> option while invoking atomsim.

Tracing can be disabled by:

::
 
  :notrace
  Trace disabled


Controlling execution
----------------------
You can advance the simulation by one clock cycle by pressing the enter-key. You can also execute 
until a desired equality is reached:

1. until value of a register <reg> becomes <value>
   :: 
     
     : until <reg> <value>

2. until value of a memory address <address> becomes <value>
   ::
   
     : until <address> <value>

3. while <condition> is true
   ::
   
     : while <condition>

4. Execute for specified number of ticks
   ::
   
     : for <ticks>
     
5. You can continue execution indefinitely by:
   ::

     : r

6. To end the simulation from the debug prompt:
   ::
   
     : q

   or
   
   ::
   
    : quit

   .. note:: 
      At any point during execution (even without -d), you can enter the interactive debug mode with :kbd:`ctrl` + :kbd:`c`.

7. Miscellaneous
   verbose-on & verbose off commands can be used to turn on /off verbosity.
