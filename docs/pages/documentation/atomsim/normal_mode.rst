Normal Mode
############

In this mode of simulation, no debug information is printed. Only serial data recieved from the system is printed to the stdout. Using ``--verbose / -v`` flag shows additional useful information.

.. code-block:: bash
   
   $ atomsim sw/examples/banner/banner.elf -v
   Input File: hello-asm/hello.elf
   Resetting..
   Relaying uart-rx to stdout (Note: This mode does not support uart-tx)
   Initialization complete!
   Hello World!
         -- from Assembly
   
   Haulting @ tick 931

Redierecting AtomSim Output
****************************
All verbose information is printed on **stderror** stream while the output of the program is printed on the **stdout**. Therefore one can easily redirect them to two different files if needed. This can be done as follows:

.. code-block:: 
   
   $ atomsim sw/examples/banner/banner.elf -v 1> output.log 2> sim.log


Reverting to Debug Mode from Normal Mode
*****************************************
During a program's execution on atomsim, if user presses :kbd:`ctrl` + :kbd:`c` atomsim quits by default. This behaviour can be changed by invoking AtomSim with ``___to_be_added____`` options. With this option, anytime during the simulation, it is possible to revert to debug mode from normal mode by pressing :kbd:`ctrl` + :kbd:`c`. To quit AtomSim, press :kbd:`ctrl` + :kbd:`c` twice or press :kbd:`ctrl` + :kbd:`c` once and then use the *quit* command on debug console.