.global _start
_start:

andi t0, t0, 0 # Clear register t0
andi t1, t1, 0 # Clear register t1
andi t2, t2, 0 # Clear register t1

li t0, 0xFF4A1A # Load register t0 with a value
mv t1, t0 # Copy contents of register t0 to register t1
mv t2, t1 # Copy contents of register t0 to register t1

nop
nop
ebreak

# $-ASSERTIONS-$
# eq t0 0x00FF4A1A
# eq t1 0x00FF4A1A
# eq t2 0x00FF4A1A
