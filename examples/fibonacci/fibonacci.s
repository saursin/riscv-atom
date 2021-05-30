.global main

.equ TX_ADDRESS, 0x00012001               
.equ TX_ACK_ADDRESS, 0x00012002               

.text

main:
    la a0, msg
    jal putstr

    li a0, 5
    jal fibonacci

    ebreak


fibonacci:
    # a0 = no of terms to print

    li s0, 0    # pprev
    li s1, 1    # prev
    li s2, 0    # curr
    mv s3, a0   # elements
    mv s4, ra   # retun address

    fib_loop:
        beq s3, x0, ret_fib
        
        add s2, s0, s1  # curr = pprev + prev
        mv s0, s1       # pprev = prev
        mv s1, s2       # prev = curr

        # print s2
        mv a0, s2
        addi a0, a0, 48
        jal putchar
        
        # print a newline
        li a0, '\n'
        jal putchar

        # decrement a0
        addi s3, s3, -1
        j fib_loop

    ret_fib:
        mv ra, s4
        ret


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
    .asciz "Fibonacci Sequence : \n"
