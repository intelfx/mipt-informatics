use64
global main
extern scanf
extern puts
extern printf

section text

main:
    mov rbp, rsp
    sub rsp, 16
    lea rdx, [rbp - 8]
    lea rax, [rbp - 16]
    mov rsi, rax
    mov rdi, read_fmt

    xor eax, eax
    call scanf

    mov rax, [rbp - 8]
    mov rbx, [rbp - 16]
    add rax, rbx

    mov rcx, over
    mov rdx, no_over
    cmovc rdx, rcx
    
    mov rdi, rdx
    call puts
    add rsp, 8

    mov rsp, rbp
    xor rax, rax
    ret 


section data
    read_fmt db "%llu %llu", 0x80
    over     db "YES",   0x00
    no_over   db "NO",    0x00  
