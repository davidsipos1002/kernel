.section .ap_init, "a", @progbits

.code16

.global __ap_init_begin
.global __ap_init_end

__ap_init_begin:
    cli
    mov %cs, %ax
    mov %ax, %ds
ap_ljmp:
    mov $(ap_count - __ap_init_begin), %bx
    addb $1, %ds:(%bx)
    hlt
ap_count:
   .skip 1
__ap_init_end:
