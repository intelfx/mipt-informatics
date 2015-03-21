use32
global main
extern scanf
extern printf

section text

main:
sub esp, 4
push esp
push read_fmt
call scanf
add esp, 8

pop eax
xor edx, edx

mov ebx, 65536
idiv ebx

push edx ; remainder
push write_fmt
call printf
add esp, 8

xor eax, eax
ret

section data

read_fmt db "%u", 0x00
write_fmt db "%u", 0x0A, 0x00