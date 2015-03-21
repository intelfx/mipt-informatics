use32
global main
extern scanf
extern puts

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
add eax, ebx

mov ecx, write_overflow
mov edx, write_no_overflow
cmovc edx, ecx

push edx
call puts
add esp, 4

xor eax, eax
ret

section data

read_fmt db "%u %u", 0x00
write_overflow db "YES", 0x00
write_no_overflow db "NO", 0x00