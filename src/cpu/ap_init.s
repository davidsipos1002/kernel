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
    // we start in real mode (16-bit, addresses are calculated like this
    // segment_reg << 4 + effective address)
    // we start here aftet SIPI from BSP, IP = VV00:000
    cli
    // copy code segment to data segment, CS is 0xVV00, according to Intel MP spec
    // we also want to address data from the vector page
    // now all adresses are relative to the 0xVV000
    mov %cs, %ax
    mov %ax, %ds

    // copy the start address of our code, set from the BSP core
    mov $(ap_vector - __ap_init_begin), %eax
    mov (%eax), %ecx 
    
    // load a temporary GDT
    lgdt (__ap_init_gdtr - __ap_init_begin)

    // enter protected mode, no paging only segmentation
    mov %cr0, %eax
    or $0x1, %eax
    // set the PE bit in CR0 to enable protection (bit 0)
    mov %eax, %cr0

    // immediately perform long jump to load the CS with descriptor from our
    // temporary GDT, BSP will edit the machine to jump to the correct, we don't know
    // where this code will be in memory
__ap_ljmp_instr:
    ljmp $0x8, $0x0000

.code32
__ap_init_ljmp:
    // now we are in 32-bit protected mode, addresses still relative to 0xVV000
    // load a data segment descriptor from the GDT
    mov $0x10, %ax
    mov %ax, %ds

    // edit GDT entry for the code segment to make it flat
    mov $0x0000FFFF, %eax
    mov %eax, (__ap_init_gdt_code - __ap_init_begin)
    mov $0x00CF9A00, %eax
    mov %eax, (__ap_init_gdt_code - __ap_init_begin + 4)

__ap_absljmp_instr:
    // far jump to load the new code segment
    ljmp $0x8, $0x00000000

__ap_absljmp:
    // load flat segment selectors for data
    mov $0x18, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss
    
    // now all addresses are flat, effective address = physical address
    // we are ready to enter IA-32e mode (aka long mode)

    // we enable PAE (bit 5 in CR4) paging, if not enabled when switching to long mode
    // we will get a general protection exception
    mov %cr4, %eax
    or $0x20, %eax
    mov %eax, %cr4
    
    // here we set the LME (long mode enable) bit (bit 8) in the IA32_EFER MSR (the extended feature 
    // model-specific register), MSR address is 0xC0000080
    mov %ecx, %ebx
    mov $0xC0000080, %ecx
    rdmsr
    or $0x100, %eax
    wrmsr

    // we saved the vector page in EBX before modifying the MSR
    mov %ebx, %ecx
    
    // the BSP core will put at ap_cr3 the physical of its own PML4 table
    // all mapped adresses by the BSP will be accessible to the AP cores
    mov %ecx, %eax
    add $(ap_cr3 - __ap_init_begin), %eax 
    mov (%eax), %ebx
    mov %ebx, %cr3
    
    // enable paging, the processor will also set LMA (long mode active) in IA32_EFER
    mov %cr0, %eax
    or $0x80000000, %eax
    // set the PG bit (bit 31) in CR0 to enable paging 
    mov %eax, %cr0

    // starting from here we are in long mode, we start by default in compatiblity mode (32-bit)
    // we do a far jump to a 64-bit code segment descriptor to enter 64-bit mode
    // again the BSP will edit the machine code to put here the correct address
__ap_ljmp64_instr:
    ljmp $0x20, $0x00000000

.code64
__ap_ljmp64:
    // now we are truly in 64-bit mode and we can access 64-bit registers
    // load the stack prepared for us the BSP core.
    mov %rcx, %rax
    add $(ap_rsp - __ap_init_begin), %rax
    mov (%rax), %rbp
    mov %rbp, %rsp
    
    // this is temporary just for testing
    // we load here a pointer to a uint8_t in RDI, the first
    // function argument in the System V ABI
    // we will increment the variable from C code to signal the BSP that we are ready
    mov %rcx, %rax
    add $(ap_count - __ap_init_begin), %rax
    mov (%rax), %rdi

    // now, we are in 64-bit mode, we have a stack and a function argument
    // we can call a C function
    mov $ap_main, %rbx
    call *%rbx

    // we should never this point
    hlt

.align 8 // Intel recommends to align the GDT at an 8-byte boundary
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
ap_count:
    .skip 8
__ap_init_end:
