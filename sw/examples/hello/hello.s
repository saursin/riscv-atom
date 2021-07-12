.global main
.text

.equ TX_ADDRESS, 0x00014001               
.equ TX_ACK_ADDRESS, 0x00014002         

main:
    la a0, msg
    jal putstr
    ebreak

putstr:
    # s0 = a0 = base address of string
    mv s0, a0

    mv s4, ra   # store return address elsewhere

    loop:
        # load a character
        lb  t0, 0(s0)
        
        # return if null
        beq t0, x0, ret_putstr

        # else print it
        mv a0, t0
        jal putchar

        # increment pointer
        addi s0, s0, 1
        j loop
    
    ret_putstr:
        mv ra, s4# restore return address
        ret


putchar:
    # a0 = character

    # t0 = tx_ack_address
    li t0, TX_ACK_ADDRESS

    # t1 = tx_address
    li t1, TX_ADDRESS

    # store character
    sb  a0, 0(t1)
    
    # toggle ack
    li  t2, 1
    sb  t2, 0(t0)
    sb  x0, 0(t0)

    ret

.data
    msg:
    .asciz "Hello World!\n      -- AtomBones\n\n"
