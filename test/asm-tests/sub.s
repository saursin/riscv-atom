 
.global _start
.text
_start:
	addi 	x1, x0, 5
	addi 	x2, x0, 7
	addi	x3, x0, 17
	add 	x2, x2, x1
	add 	x2, x2, x1
	add     x2, x2, x3   # x2 = 34 (0x22)
	
	sub 	x5, x2, x3  
	sub 	x7, x2, x1 

	addi	x8, x7, -12 

	sub     x9, x8, x5 # x9 = 0 (0)
	addi		x9, x9, -1	# x9 = -1
