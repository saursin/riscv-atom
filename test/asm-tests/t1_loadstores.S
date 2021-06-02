.globl _start

.text
_start:
    # Test Byte Loads
    li t1, 43
    la t1, mydata
    
    lb  t2, 3(t1)   # t2 = FFFFFF87
    lbu t3, 3(t1)   # t3 = 00000087
    lh  t4, 2(t1)   # t4 = FFFF8765
    lhu t5, 2(t1)   # t5 = 00008765

    li  t2, 0xffffffff
    sb  t2, 4(t1)
    sh  t2, 5(t1)
    sw  t2, 7(t1)


    
       
    


_exit:
    ebreak


.data
    mydata: .word 0x87654321



