.globl _start

.text
_start:
    li x0, 0
    li x0, 1
    li x0, 2
    # Test Byte Loads
    la t1, mydata
    lb t1, 0(t1)
      
    


_exit:
    ebreak


.data
    mydata: .word 0x87654321



