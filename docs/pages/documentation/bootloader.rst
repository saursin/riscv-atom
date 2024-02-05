RISC-V Atom Bootloader
#######################

The RISC-V Atom bootloader is a **stage-0** bootloader. It is loaded in the BootROM and executable as the first thing
after reset. The source code for Atom bootloader is located in ``RVATOM/sw/bootloader`` subdirectory. The bootloader can
be built manually using the following *make* command.

.. code-block::

    $ cd sw/bootloader
    $ make soctarget=hydrogensoc

During run-time, the bootloader uses RAM to implement stack. it includes code for platform initialization, loading
executable and control transfer. Bootloader provides various *bootmodes* to the user allowing the user to control the
boot process.

Following is a list of actions that are performed step-by-step during the boot process:

#. Upon reset the control is transferred to reset vector, which by default points to start of BootROM.
#. CPU starts executing the stage-0 bootloader.
    #. Bootloader performs common initialization.
    #. Bootloader then performs any platform specific initializations.
    #. Bootloader reads the *bootmode* pins and decides the bootmode.
    #. Actions are performed according to bootmode, which includes loading the executable into RAM from correct source.
    #. Finally, the control is transferred to User program.


Bootmodes
**********

RISC-V Atom bootloader mainly supports 4 boot modes:

+----------+---------------+----------------------------------------------------------------------+
| Pinval   | Bootmode      | Function                                                             |
+==========+===============+======================================================================+
| ``0b00`` | FLASHBoot     | Loads a binary image into RAM from FLASH memory using SPI            |
+----------+---------------+----------------------------------------------------------------------+
| ``0b01`` | Jump to RAM   | Jumps to RAM, without memory initialization                          |
+----------+---------------+----------------------------------------------------------------------+
| ``0b10`` | UARTBoot      | Loads a binary image into RAM from UART stream using XMODEM protocol |
+----------+---------------+----------------------------------------------------------------------+
| ``0b11`` | Infinite Loop | Executes an infinite loop                                            |
+----------+---------------+----------------------------------------------------------------------+

FLASHBoot
==========
In this bootmode, user must write the binary image for the program directly in FLASH memory at a predefined offset value.
During boot, this binary image will be copied as it is to the RAM and will be executed. FLASHBoot requires the SPI IP to
communicate with FLASH memory and will give an boot-panic error if its missing.

UARTBoot
=========
In this mode, the bootloader obtains the program binary image from UART using the 
`XMODEM protocol <https://en.wikipedia.org/wiki/XMODEM>`_. It supports XMODEM CRC16 with default packet size of 128 bytes.

Transferring files over UART
-----------------------------
RISC-V Atom project provides the ``xmsend.py`` python script that can be used to transmit binary files over UART. *xmsend*
can be used as follows. 

.. code-block:: bash

    $ xmsend.py -b 115200 -p /dev/ttyUSB0 firmware.bin

.. tip::
    Make sure that no other process (like screen) is using the serial port before invoking the xmsend script. 
