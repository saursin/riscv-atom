.global _start
_start:

lui a0, 0xfffff
lui a1, 0xf210a
lui a2, 0x00001
lui a3, 0x00123
lui a4, 0x01234
lui a5, 0xdeadb
lui a6, 0xdbeef
lui a7, 0xae121

lui t0, 0x52cda
lui t1, 0xcda53
lui t2, 0xab129
lui t3, 0x95bcd
lui t4, 0x09090
lui t5, 0xff11f
lui t6, 0x96dad

nop
nop
ebreak
