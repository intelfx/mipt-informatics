use32
global main
extern scanf
extern printf

section text

main:
sub esp, 8
push esp
push read_fmt
call scanf
add esp, 8

fld qword [esp]
fabs
fstp qword [esp]

push write_fmt
call printf
add esp, 12

xor eax, eax
ret

section data

read_fmt db "%lf", 0x00
write_fmt db "%lf", 0x0A, 0x00