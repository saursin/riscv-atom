 
.global _start
.text
_start:
	auipc x1, 0x0
	auipc x1, 0x0
	auipc x1, 0x0
	auipc x1, 0x0	# actual pc & reg value must match till here
	
	auipc x1, 0x4
	auipc x1, 0x4
	auipc x1, 0x4
	auipc x1, 0x4	# actual pc & reg value must match till here

	auipc x2, 0x0
