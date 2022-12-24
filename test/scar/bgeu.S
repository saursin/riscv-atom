.global _start
_start:
li x10, 1
li x11, 2
li x12, 3
li x13, 4
li x14, 5
li x15, 6

#---1
li x1, 500
li x2, 500
bgeu x1, x2, pass1
li x10, 0
pass1:

li x1, 400
li x2, 300
bgeu x1, x2, pass2
li x11, 0

pass2:
li x1, 400
li x2, 399
bgeu x1, x2, pass3
li x12, 0

pass3:  # signed check for unsigned
li x1, -2
li x2, 0
bgeu x1, x2, pass4
li x13, 0

pass4:
li x1, 100
li x2, 0
bgeu x1, x2, pass5
li x14, 0

pass5:
li x1, 12
li x2, 12
bgeu x1, x2, pass6
li x15, 0

pass6:
ebreak

# $-ASSERTIONS-$
# eq x10 0x00000001
# eq x11 0x00000002
# eq x12 0x00000003
# eq x13 0x00000004
# eq x14 0x00000005
# eq x15 0x00000006

