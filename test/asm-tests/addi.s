.global _start
.text
_start:
	addi x1, x0, 2
	addi x2, x1, 2
	addi x3, x2, 2
	addi x4, x3, 2
	addi x5, x4, 2
	addi x6, x5, 2
	addi x7, x6, 2


	# construct 0x12345678 in x7
	lui x7, 0x12345
	addi x7, x7, 0x678
