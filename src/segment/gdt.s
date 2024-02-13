.section .bss
 __gdtr:
    .skip 10

.section .text
.global __set_gdt
__set_gdt:
    mov %rdi, (__gdtr + 2)
    mov %si, (__gdtr)
    lgdt (__gdtr)
    ret

.global __set_segment_regs
__set_segment_regs:
    sub $16, %rsp
    movq $8, 8(%rsp)
    movabsq $__reload_cs, %rax
    mov %rax, (%rsp)
    retfq
__reload_cs:
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss
    ret
