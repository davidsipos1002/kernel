/*
    Entry code for kernel, it sets up an initial stack (according to the Intel manuals stack needs to be 16KB aligned
    for C and calls kernel_main
*/

/* 
    Placing stack in the .bss section, because it will not occupy space in the ELF file.
    The bootloader will allocate the stack based on the program header
*/
.section .bss
.global __kernel_data_begin
.global __kernel_data_end
.align 16
//stack grows downwards for x86
__stack_end:
.skip 8388608
__stack_begin:

__kernel_data_begin:
.skip 33554432
__kernel_data_end:

/*
    __kernel_start placed in the code section
    According to the UEFI specification the handoff state for the bootloader is the following:
    Mode of operation: IA-32e 64-bit mode (also called long mode)
    Paging enabled, everything identity mapped
    64-bit code segment loaded
    The bootloader sets up 4-level recursive page tables and maps the kernel
    and BootInfo structure (it's address is in %rdi). The framebuffer and memory map are not mapped 
    their physical address can be found in the BootInfo.
    The GDT, IDT, page tables and other control registers will be properly set up from the C code.
*/
.section .text
.global __kernel_start
__kernel_start:
    cli

    xor %rax, %rax
    xor %rbx, %rbx
    xor %rcx, %rcx
    xor %rdx, %rdx
    xor %rsi, %rsi
    xor %r8, %r8
    xor %r9, %r9
    xor %r10, %r10
    xor %r11, %r11
    xor %r12, %r12
    xor %r13, %r13
    xor %r14, %r14
    xor %r15, %r15

    lea __stack_begin(%rip), %rbp
    lea __stack_begin(%rip), %rsp

    call kernel_main

    cli
    hlt
end_loop:
    jmp end_loop

/*
    According to the GNU as manual:
        This directive is generated by compilers to include auxiliary debugging information in the symbol table.
*/
.size __kernel_start, . - __kernel_start
