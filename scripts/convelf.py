#!/usr/bin/python

import sys
import textwrap
import subprocess
from colorama import Fore, Back, Style

# ConvELF : A handy tool to convert ELF files to simulation friendly verilog 
# $readmemh/$readmemb format files

class ConvELF:
    # @param infile input elf file
    # @param map memory map
    def __init__(self, infile, map):
        self.__input_elf_file = infile
        self.__memory_map = map
        
        # RISCV Toolchain Prefix
        self.RVPREFIX = 'riscv64-unknown-elf-'

        # Sections to load into hex files
        self.INCLUDE_SECTIONS = ['.text', '.rodata', '.sdata', '.data']

        # Name of temporary hex file
        self.TEMP_FILE = 'temp.hex'

        # Echo shell commands during execution
        self.ECHO_CMD = False


    def __runcommand(self, command, echo=True):
        if echo:
            for c in command:
                print(c+' ', end = '')
            print()

        # Execute shell command using python subprocess module
        dump = subprocess.run(
            command , capture_output=True, text=True
        )

        # dump stderr & stdout if their length is non zero
        if len(dump.stderr) !=0:
            print(dump.stderr)
            sys.exit()

        if len(dump.stderr) !=0:
            print(dump.stderr)
            sys.exit()


    def __genTempFile(self):
        # prepare objcopy command
        command = [self.RVPREFIX+'objcopy', '-O', 'verilog']

        for sec in self.INCLUDE_SECTIONS:
            command+=['-j', sec]
        
        command+=['--reverse-bytes=4', '--verilog-data-width','4', self.__input_elf_file, self.TEMP_FILE]
        
        # Execute command to generate tempfile
        self.__runcommand(command, self.ECHO_CMD)
        


    def __rmTempFile(self):
        # execute command to remove tempfile
        self.__runcommand(['rm', '-f', self.TEMP_FILE], self.ECHO_CMD)



    def convert(self, outformat = 'hex'):
        # ===== Step-1 =====
        # use objcopy to generate a verilog memory instantiation file : temp.hex
        self.__genTempFile()


        # ===== Step-2 =====
        # parse and convert file

        # open tempfile
        tfile = open(self.TEMP_FILE, 'r')

        # get device names from map
        mapkeys = self.__memory_map.keys()
        
        # create a dummy file handle 
        # TODO: find a more efficient way to create a variable of type file.
        ofile = open(self.TEMP_FILE, 'r')
        ofile.close()

        # Declare a variable to keep track of current address
        currAddr = 0x00000000

        # Declare a variable to keep track of the device file curently writing to
        currKey = 'None'

        # iterate overr lines of tempfile
        for lineno, line in enumerate(tfile):

            # strip all leading and trailing whitespaces
            line = line.strip()
            
            # skip blank lines
            if len(line) == 0:
                continue
            
            # detect address markers & accordingly update currAddr
            if '@' in line:
                currAddr = int(line.replace('@', ''), 16)
                continue

            
            # Wrap line to 4 bytes width
            line=textwrap.wrap(line, 4*2)

            for i in range(len(line)):

                # if CurrAddr is in address range of a device, switch to its corresponding file
                for k in mapkeys:
                    if (currAddr >= self.__memory_map[k][0]) and (currAddr < self.__memory_map[k][0]+self.__memory_map[k][1]) and (currKey != k):
                        currKey = k

                        # close if already open
                        if not ofile.closed:
                            ofile.close()
                        
                        # open file for writing
                        ofile = open(self.__memory_map[k][2], 'w')

            
                # write line
                ofile.write(line[i])
                ofile.write('\n')

                # increment current address
                currAddr+=1
        
        # close all files
        ofile.close()
        tfile.close()


        # ===== Step-3 ===== 
        # Remove temp file
        memImg = self.__rmTempFile()



if __name__ == "__main__":
    # arg check
    if len(sys.argv) != 2:
        print(Fore.RED+"ERROR!: "+Style.RESET_ALL+"Wrong number of args\n Usage: $ python3 gen_usable_hex.py [inputFile]")
        sys.exit()

    # === Memory Map Format ===
    #   Device  | ORIGIN    |   SIZE    |   HEXFILE NAME
    HydrogenSoC_MemMap = {
        'IMEM' : [0x00000000, 0x00008000, 'build/init/code.hex'],
        'DMEM' : [0x04000000, 0x00002000, 'build/init/data.hex']
    }

    # Create a convelf object.
    ce = ConvELF(sys.argv[1], HydrogenSoC_MemMap)

    # generate output hex files.
    ce.convert('hex')
