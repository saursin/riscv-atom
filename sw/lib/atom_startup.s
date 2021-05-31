# Simple Bootloader for c programs in atomRV core
.globl _start

.equ __SP_INIT, 0x00fc00

.text
_start:
    # ===== initialization =====
    li sp, __SP_INIT    # set stack pointer

    # ===== Callin main =====
    jal main

_exit:
    ebreak

main:
