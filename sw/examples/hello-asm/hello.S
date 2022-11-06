.global main
.text

#ifdef TARGET_ATOMBONES
.equ TX_ADDRESS, 0x08000000
#else
.equ TX_ADDRESS, 0x40000000
#endif

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

        # otherwise print it
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

    # t1 = tx_address
    li t1, TX_ADDRESS

    # store character
    sb  a0, 0(t1)
    ret

.data
    msg:
    .asciz "Hello World!\n      -- from Assembly\n\n"
