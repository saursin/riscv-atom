.global _start
.text
_start:
	addi 	x1, x0, 5
	addi 	x2, x0, 7
	addi	x3, x0, 17
	add 	x2, x2, x1
	add 	x2, x2, x1
	add     x2, x2, x3   # x2 = 34 (0x22)
	