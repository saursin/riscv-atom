.global _start
_start:
    # Stores
    la x31, memloc

    li x1, 0x12345678
    sw x1, 0(x31)
    addi x31, x31, 4

    li x1, 0x00002020
    sw x1, 0(x31)
    addi x31, x31, 4

    li x1, 0xf3232532
    sw x1, 0(x31)
    addi x31, x31, 4

    li x1, 0xdea14245
    sw x1, 0(x31)
    addi x31, x31, 4

    li x1,0xdea12cad
    sw x1, 0(x31)
    addi x31, x31, 4

    li x1, 0xdacadaca
    sw x1, 0(x31)
    addi x31, x31, 4

    li x1, 0xbeefdead
    sw x1, 0(x31)
    addi x31, x31, 4

    li x1, 0xdeadbeef
    sw x1, 0(x31)
    addi x31, x31, 4

    li x1, 0x1beefbee
    sw x1, 0(x31)
    addi x31, x31, 4

    li x1, 0x12deadbe
    sw x1, 0(x31)
    addi x31, x31, 4

    li x1, 0x123deadb
    sw x1, 0(x31)
    addi x31, x31, 4

    li x1, 0x1234dead
    sw x1, 0(x31)
    
    # Loads
    la x31, memloc

    lw x1, 0(x31)
    addi x31, x31, 4

    lw x2, 0(x31)
    addi x31, x31, 4

    lw x3, 0(x31)
    addi x31, x31, 4

    lw x4, 0(x31)
    addi x31, x31, 4

    lw x5, 0(x31)
    addi x31, x31, 4

    lw x6, 0(x31)
    addi x31, x31, 4

    lw x7, 0(x31)
    addi x31, x31, 4

    lw x8, 0(x31)
    addi x31, x31, 4

    lw x9, 0(x31)
    addi x31, x31, 4

    lw x10, 0(x31)
    addi x31, x31, 4

    lw x11, 0(x31)
    addi x31, x31, 4

    lw x12, 0(x31)
    addi x31, x31, 4


    ebreak


.data
memloc:
