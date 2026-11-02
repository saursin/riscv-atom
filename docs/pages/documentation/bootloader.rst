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
    info                    SoC summary
    clear
    md <addr> [words]       dump
    mw <addr> <val>         write
    fill <addr> <len> <val>
    crc <addr> <len>        CRC-16
    memtest <addr> <len>
    ddr                     DDR state
    load [addr]             XMODEM in
    save <addr> <len> [name] XMODEM out
    flashboot
    boot [addr]             run
    reboot

``load`` receives a file over XMODEM (default address: program RAM), ``save`` sends memory as a file, ``crc`` is
CRC-16/XMODEM, ``memtest`` an address-in-address test, ``ddr`` the DDR calibration state (boards with DDR).
Numbers are decimal, or hex with ``0x``. Connect with ``atomterm`` (below) or any serial terminal.

When a program started by the bootloader exits, the bootloader takes over again and opens the console (memory,
including DDR, is left as the program left it), so its results can be read with ``md`` or sent with ``save``.

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

Commands start with ``Ctrl-T``, followed by ``q`` (quit), ``u`` (upload a file), ``c`` (cancel a transfer) or ``h``
(help); ``Ctrl-T Ctrl-T`` sends a ``Ctrl-T``. All other keys go to the SoC. ``atomterm`` requires *pyserial*.

At the console, ``atomterm`` can load several files and start the program, e.g. an image into DDR and the program
into RAM:

.. code-block:: bash

    $ atomterm /dev/ttyUSB0 --load 0x80000000:image.raw --load prog.bin --run

Files the SoC sends (the console's ``save``, or a program calling ``xmodem_send()`` from ``xmodem.h``) are received
automatically and saved under the name they were sent with, in ``--save-dir`` (an existing file is never
overwritten). Both directions are announced with a marker on the console (``![XMODREQ]!(1k)``,
``![XMODSND]!(<len>,<name>)``), which ``atomterm`` watches for.

.. tip::
    Make sure that no other process (like screen) is using the serial port before starting atomterm.

    Without a port, ``atomterm`` connects to AtomSim run with ``-p`` (``$RVATOM/sim.tty``), so everything above works
    the same in simulation.
