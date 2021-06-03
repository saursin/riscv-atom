.global _start
_start:

andi t0, t0, 0 # Clear register t0
andi t1, t1, 0 # Clear register t1
li t0, 0x00011000 # Load register t0 with an address
li t1, 0x7B7A7971 # Load register t1 with a 4-Byte (1 word) value
sw t1, 0(t0) # Store the word in t1 to the first-word slot of address specified in t0
li t1, 0x7F7E7D7C # Load register t1 with another 4-Byte (1-word) value
sw t1, 4(t0) # Store the word in t1 to the second word slot of address specified in t0

nop
nop
ebreak