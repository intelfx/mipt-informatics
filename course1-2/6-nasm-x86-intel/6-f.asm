use32
global main
extern scanf
extern printf

section text

main:
sub esp, 8
mov eax, esp
push eax
add eax, 4
push eax
push read_fmt
call scanf
add esp, 12

pop eax
pop ebx
cmp eax, ebx
cmovg eax, ebx

push eax
push write_fmt
call printf
add esp, 8

xor eax, eax
ret

section data

read_fmt db "%d %d", 0x00
write_fmt db "%d", 0x0A, 0x00