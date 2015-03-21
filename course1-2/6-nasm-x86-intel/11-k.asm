use32
global main
extern scanf
extern printf

section text

main:
sub esp, 16
mov eax, esp
push eax
add eax, 8
push eax
push read_fmt
call scanf
add esp, 12

fld qword [esp]
add esp, 8
fdivr qword [esp]
fstp qword [esp]

push write_fmt
call printf
add esp, 12

xor eax, eax
ret

section data

read_fmt db "%lf %lf", 0x00
write_fmt db "%lf", 0x0A, 0x00