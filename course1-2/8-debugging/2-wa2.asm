global main
extern printf
extern scanf

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

pop ebx
pop eax
sub eax, ebx

push eax
push print_fmt
call printf
add esp, 8

xor eax, eax
ret

section .data
scan_fmt db "%d %d", 0x00
print_fmt db "%d", 0x0A, 0x00