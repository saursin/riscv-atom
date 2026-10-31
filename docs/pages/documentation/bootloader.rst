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
| ``0b11`` | Console       | Interactive console (see below)                                      |
+----------+---------------+----------------------------------------------------------------------+

If FLASHBoot or UARTBoot fails (no flash device, XMODEM timeout or error), the bootloader falls back to the console
instead of stopping.

FLASHBoot
==========
In this bootmode, user must write the binary image for the program directly in FLASH memory at a predefined offset value.
During boot, this binary image will be copied as it is to the RAM and will be executed. Without the SPI IP, or without
a flash device, the bootloader falls back to the console.

Console
========
A small console in the boot ROM. On entry it prints a SoC summary (ISA from ``misa``, clock, memories, peripherals)
and then takes commands:

.. code-block:: text

    atom-boot: help
    help                  this list
    info                  SoC summary
    clear                 clear the screen
    md <addr> [words]     dump memory
    mw <addr> <value>     write a word
    fill <addr> <len> <v> fill memory with a word
    crc <addr> <len>      CRC-16/XMODEM of memory
    memtest <addr> <len>  address-in-address test
    ddr                   DDR size and calibration (boards with DDR)
    uartboot              receive a program (XMODEM) and run it
    flashboot             load a program from flash and run it
    boot [addr]           run the program in RAM (or at addr)
    reboot

Numbers are decimal, or hex with ``0x``. Connect with ``atomterm`` (below) or any serial terminal.

UARTBoot
=========
In this mode, the bootloader obtains the program binary image from UART using the 
`XMODEM protocol <https://en.wikipedia.org/wiki/XMODEM>`_. It supports XMODEM CRC16 with default packet size of 128 bytes.

Transferring files over UART
-----------------------------
RISC-V Atom provides ``atomterm``, a serial console (similar to *litex_term*) that also uploads binaries to the
bootloader. With ``--bin``, the file is sent automatically when the bootloader asks for it, and the session continues
as a console, so the program's output appears in the same terminal.

.. code-block:: bash

    $ atomterm /dev/ttyUSB0 -b 115200 --bin firmware.bin

Press ``Ctrl-]`` to exit and ``Ctrl-U`` to upload the ``--bin`` file manually. ``atomterm`` requires *pyserial*.

.. tip::
    Make sure that no other process (like screen) is using the serial port before starting atomterm.

    In simulation (AtomSim with a virtual UART), the bootloader cannot keep up with an unpaced upload; add
    ``--byte-delay 0.005``.
