.global _start
_start:
andi t0, t0, 0 # Clear register t0
andi t1, t1, 0 # Clear register t1
andi t1, t1, 0 # Clear register t1
li t0, 0x10011000 # Load register t0 with an address
li t1, 0x7F7E7D7C7B7A7971 # Load register t1 with double word (8-bytes = 2 words) value


nop
nop
ebreak