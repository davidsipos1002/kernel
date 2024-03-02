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
ap_cr3: // physical address of PML4
    .skip 8
ap_gdt: // gdt base and limit
    .skip 10
ap_idt: // idt base and limit
    .skip 10
ap_tss: // tss selector
    .skip 2
ap_count:
   .skip 1
__ap_init_end:
