.global __spinlock_get
.global __spinlock_release

// Taken from Intel manual
spin:
    cmpq $0, (%rdi) // check if lock is free
    je __spinlock_get
    pause // short delay
    jmp spin
__spinlock_get:
    mov $1, %rax
    xchg %rax, (%rdi) // try to get lock
    cmp $0, %rax // test if successful
    jne spin
    ret

__spinlock_release:
    xor %rax, %rax
    xchg %rax, (%rdi)
    ret
