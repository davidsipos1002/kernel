.section .ap_init, "a", @progbits

.global __ap_init_begin
.global __ap_init_end
.global __ap_ljmp_instr
.global __ap_init_ljmp
.global __ap_init_gdt
.global __ap_init_gdtr
.global __ap_absljmp_instr
.global __ap_absljmp
.global __ap_ljmp64_instr
.global __ap_ljmp64
.global __ap_init_protected

.code16
__ap_init_begin:
    cli
    // copy code segment to data segment, cs is 0xVV00, according to Intel MP spec
    // we also want to address from the vector page
    mov %cs, %ax
    mov %ax, %ds

    mov $(ap_vector - __ap_init_begin), %eax
    mov (%eax), %ecx 
    
    lgdt (__ap_init_gdtr - __ap_init_begin)
    mov %cr0, %eax
    or $0x1, %eax
    mov %eax, %cr0
__ap_ljmp_instr:
    ljmp $0x8, $0x0000

.code32
__ap_init_ljmp:
    mov $0x10, %ax
    mov %ax, %ds

    mov $0x0000FFFF, %eax
    mov %eax, (__ap_init_gdt_code - __ap_init_begin)
    mov $0x00CF9A00, %eax
    mov %eax, (__ap_init_gdt_code - __ap_init_begin + 4)

__ap_absljmp_instr:
    ljmp $0x8, $0x00000000

__ap_absljmp:
    mov $0x18, %ax
    mov %ax, %ds
    
    mov %cr4, %eax
    or $0x20, %eax
    mov %eax, %cr4
    
    mov %ecx, %ebx
    mov $0xC0000080, %ecx
    rdmsr
    or $0x100, %eax
    wrmsr
    rdmsr
    mov $(ap_test - __ap_init_begin), %edx
    add %ebx, %edx
    mov %eax, (%edx)
    mov %ebx, %ecx
    
    mov %ecx, %eax
    add $(ap_cr3 - __ap_init_begin), %eax 
    mov (%eax), %ebx
    mov %ebx, %cr3
    
    mov %cr0, %eax
    or $0x80000000, %eax
    mov %eax, %cr0

__ap_ljmp64_instr:
    ljmp $0x20, $0x00000000

.code64
__ap_ljmp64:
    mov %rcx, %rax
    add $(ap_count - __ap_init_begin), %rax
    addb $1, (%rax)
    hlt

.align 8 // Intel says align at 8-byte boundary
__ap_init_gdt:
    .long 0, 0
__ap_init_gdt_code:
    .long 0x0000FFFF, 0x00CF9A00 // selector is 0x8
__ap_init_gdt_data:
    .long 0x0000FFFF, 0x00CF9200 // selector is 0x10
__ap_init_gdt_data2:
    .long 0x0000FFFF, 0x00CF9200 // selector is 0x18
__ap_init_gdt_code64:
    .long 0x0000FFFF, 0x00AF9A00 // selector is 0x20

__ap_init_gdtr:
    .word 0x31 
__ap_init_gdtr_base:
    .long 0

__ap_init_gdtrr:
    .word 0
    .long 0

.align 8
__ap_init_protected:
    .long 0 // jump address
    .word 0x8 // segment selector

.skip 8
ap_cr3: // physical address of PML4
    .skip 4
ap_gdt: // gdt base and limit
    .skip 10
ap_idt: // idt base and limit
    .skip 10
ap_rsp:
    .skip 8
ap_vector:
    .skip 4
ap_test:
    .skip 4
ap_count:
    .skip 1
__ap_init_end:
