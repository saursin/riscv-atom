.global _start
_start:

li t0, 0x00091E65	#597605
li t1, 0x000009EF	#2543
li t2, 0x00625370	#6443888
li t3, 0x00002179	#8569
li t4, 0x0		#0
li t5, 0x00003100	#12544

divu a0, t0, t1
divu a1, t2, t3
divu a2, t4, t5

nop
nop
ebreak


