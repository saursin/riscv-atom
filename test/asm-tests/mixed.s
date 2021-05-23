.global _start
_start: 
    lui     x1, 0x65
    lui     x2, 0x87
    addi    x1, x1, 0x432   # x1 = 0x65432
    addi    x2, x2, 0x654   # x2 = 0x87654

    xor    x1, x1, x2  # x1 = 0xe2266
    or     x2, x1, x2 # x2 = 0xe7676
    
    xori    x1, x1, -1      # x1 =  0xfff1dd99
    ori     x2, x2, 0xff    # x2 = 0xe76ff
    
    addi    x0, x0, 0

loop:
    lui     x4, 0x123
    lui     x5, 0x123
    addi    x4, x4, 0x456
    addi    x5, x5, 0x456
    beq     x4, x5, loop
    
