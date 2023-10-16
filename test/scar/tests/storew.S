.global _start
_start:

la t0, _sdata   # Load register t0 with an address

li t1, 0x9B7A7971   # Load register t1 with a 4-Byte (1 word) value
li t2, 0x4523521d   # Load register t1 with a 4-Byte (1 word) value
li t3, 0x73451464   # Load register t1 with a 4-Byte (1 word) value
li t4, 0xf4323513   # Load register t1 with a 4-Byte (1 word) value

sw t1, 0(t0)        # Store the word in t1 to the first-word slot of address specified in t0
sw t2, 4(t0)        # Store the word in t1 to the second word slot of address specified in t0
sw t3, 8(t0)        # Store the word in t1 to the second word slot of address specified in t0
sw t4, 12(t0)        # Store the word in t1 to the second word slot of address specified in t0

lw a1, 0(t0)        # Store the word in t1 to the first-word slot of address specified in t0
lw a2, 4(t0)        # Store the word in t1 to the second word slot of address specified in t0
lw a3, 8(t0)        # Store the word in t1 to the second word slot of address specified in t0
lw a4, 12(t0)        # Store the word in t1 to the second word slot of address specified in t0


nop
nop
ebreak
