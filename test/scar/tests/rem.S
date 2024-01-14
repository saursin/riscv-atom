.global _start
_start:

li t0, 0x000009EF	#2543
li t1, 0x000000EB	#235
li t2, 0x00002179	#8569
li t3, 0x000002F0	#752
li t4, 0x0		#0
li t5, 0x00003100	#12544

rem a0, t0, t1
rem a1, t2, t3
rem a2, t4, t5

nop
nop
ebreak
