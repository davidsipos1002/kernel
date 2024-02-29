.section .ap_init, "a", @progbits

.code16

.global __ap_init_begin
.global __ap_init_end

__ap_init_begin:
    cli
    mov $0x0000, %ax
    mov %ax, %ds
    ljmp $0x0000, $(ap_ljmp - __ap_init_begin) 
ap_ljmp:
    mov $10, %ebx
    mov $(ap_ljmp - __ap_init_begin), %ax
    jmp *%ax
    hlt

__ap_init_end:
