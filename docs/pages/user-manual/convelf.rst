*********
ConvELF 
*********
ConvELF is a flexible tool written in python to convert ELF executable files to Verilog friendly 
$readmemh/$readmemb compatible files. ConvELF is basically a target-configurable tool which can be 
configured by specifying a memory map. `convelf.py` can be either imported into user programs as a 
module or user may directly use it as a script.

convelf.py as a Python Script
==============================
convelf.py can be invoked as a script by passing elf file as a command-line argument.

::
  
  $ python3 convelf.py file.elf


convelf.py as a Python Module
==============================
Using ConvELF is easy, just import it into your code and instantiate the ConvELF class.

::
    
  import convelf as CE

  # Create a convELF class object
  ce = CE.ConvELF(ELF_file, Memory_Map)

Now, Generate $readmemh compatible files, using:
::

  # generate output hex files for $readmemh.
  ce.convert('hex')


alternatively to generate $readmemb compatible files, using:
::

   # generate output bin files for $readmemh.
   ce.convert('hex')


ConvELF provides the following object fields to further configure the tool
::
  
  # RISCV Toolchain Prefix
  ce.RVPREFIX = 'riscv64-unknown-elf-'
  
  # Sections to load into hex files
  ce.INCLUDE_SECTIONS = ['.text', '.rodata', '.sdata', '.data']
  
  # Name of temporary hex file
  ce.TEMP_FILE = 'temp.hex'
  
  # Echo shell commands during execution
  ce.ECHO_CMD = False
  
  # Delete temp file after conversion
  ce.DELETE_TEMPFILE = True
  

Memory Map Format
==================
Memory map of the target should be specified in python dictionary format.
one key-value pair (element) in the dictionary must represent one memory block for which a 
initialization file should be generated. 

- The **key** should be a string which should represent the name of the memory block.
- The **value** should be a python list containing atleast 3 elements. 
  
  1. **base address** of the memory block.
  2. **size** of the memory block.
  3. **name of the initialization file** to be generated.


Example
=========
Let's assume a target that has 3 different memories.

.. list-table:: Example Memory Map
   :widths: 25 25 25 25
   :header-rows: 1

   * - Memory
     - Base Address
     - Size
     - Init file
     
   * - ROM
     - 0x00000000
     - 0x00010000
     - boot.hex

   * - FLASH
     - 0x01000000
     - 0x00010000
     - code.hex
 
   * - EEPROM
     - 0x02000000
     - 0x00010000
     - data.hex

This memory map can be specified in python as:

::

  HydrogenSoC_MemMap = {
      'ROM'   : [0x00000000, 0x00001000, 'boot.hex'],
      'FLASH' : [0x01000000, 0x00001000, 'code.hex'],
      'EEPROM': [0x02000000, 0x00001000, 'data.hex']
  }

