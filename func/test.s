.intel_syntax noprefix
.globl main
main:
    push 4
    push 3
    pop rdi
    pop rsi

    mov rax, rsp
    and rax, 15
    cqo
    idiv rdi
    cmp rdx, 0
    je .Lend
    sub rsp, 8
    call bar

    add rsp, 8
    