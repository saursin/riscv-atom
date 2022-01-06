.global main
.text

.global _start

main:
    csrrw t0, cycle, x0 // read time 

    li t2, 0
    li t3, 10
loop:
    beq t2, t3, exitloop
    addi t2, t2, 1
    j loop
exitloop:

    csrrw t1, cycle, x0 // read time 
    
    // get difference 
    sub t1, t1, t0

    ebreak

