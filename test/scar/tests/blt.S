.global _start
_start:
li x10, 1
li x11, 2
li x12, 3
li x13, 4
li x14, 5
li x15, 6

#---1
li x1, 50
li x2, 500
bltu x1, x2, pass1
li x10, 0
pass1:

li x1, -5 # signed check for unsigned
li x2, -4
bltu x1, x2, pass2
li x11, 0

pass2:
li x1, 0
li x2, 100
bltu x1, x2, pass3
li x12, 0

pass3:  # signed check for unsigned
li x1, 0
li x2, -1
bltu x1, x2, pass4
li x13, 0

pass4:
li x1, 11
li x2, 12
bltu x1, x2, pass5
li x14, 0

pass5:
li x1, 12
li x2, 121
blt x1, x2, pass6
li x15, 0

pass6:
ebreak
