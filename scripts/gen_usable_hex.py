#!/usr/bin/python

import sys
import textwrap

def genUsableHex(infile, outfilesPath, seperateInstrDataMemory = False):
    ifile = open(infile, 'r')
    ofile = open(outfilesPath+'/code.hex', 'w')

    wrote_Dmem = False
    for lineno,line in enumerate(ifile):
        if(line.strip() == '@04000000') and seperateInstrDataMemory:
            ofile.close()
            ofile=open(outfilesPath+'/data.hex','w')
            wrote_Dmem = True
            continue
        elif '@' in line:
            ofile.write(line+"\n")
            continue

        line=textwrap.wrap(line,8)
        for i in range(len(line)):
            ofile.write(line[i])
            ofile.write("\n") 

    # In case there was nothing to push to dmem
    if seperateInstrDataMemory and not wrote_Dmem:
        ofile=open(outfilesPath+'/data.hex','w')
        ofile.write("00000000\n")


    ifile.close()
    ofile.close()




KB = 1024
MB = KB*1024

memMap = {
    'imem': [0x00000000, 32*KB],
    'dmem': [0x04000000, 8*KB]
    }


if len(sys.argv) != 2:
    print("Wrong number of args")
    sys.exit()

genUsableHex(sys.argv[1], '/home/frozenalpha/git/riscv/riscv-atom/build/init', True)
