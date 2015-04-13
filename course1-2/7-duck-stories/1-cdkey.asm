global main
extern scanf
extern puts

section .text

main:
sub esp, 8
mov eax, esp
push eax
add eax, 4
push eax
push scan_fmt
call scanf
add esp, 12

pop eax
pop ebx
add eax, ebx
cmp eax, 42

mov eax, accepted_message
mov edx, wrong_message
cmovne eax, edx

push eax
call puts
add esp, 4

xor eax, eax
ret

section .data
scan_fmt db "%d %d", 0x00
accepted_message db "Accepted!", 0x00
wrong_message db "Wrong key!", 0x00