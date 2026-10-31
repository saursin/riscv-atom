.global main
.text

.equ TX_ADDRESS, SOC_UART_ADDR

main:
    la a0, msg
    jal putstr
    j _exit             # libcatom: flush UART, exit hint, then spin

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

    # wait for space in the TX FIFO (LSR bit 1)
1:  lw t2, 8(t1)
    andi t2, t2, 2
    beqz t2, 1b

    # store character
    sb  a0, 0(t1)
    ret

.data
    msg:
    .asciz "Hello World!\n      -- from Assembly\n\n"
