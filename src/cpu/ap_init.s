.section .ap_init

.code16

ap_start:
    cli
ap_loop:
    xor %ax, %ax
    jmp ap_loop
