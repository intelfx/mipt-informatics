global main
extern scanf
extern printf

section .text

main:
sub esp, 4
push esp
push scan_fmt
call scanf
add esp, 8

pop eax

xor eax, 'Genu'
xor eax, 'ineI'
xor eax, 'ntel'

xor eax, 'Auth'
xor eax, 'enti'
xor eax, 'cAMD'

push eax
push print_fmt
call printf
add esp, 8

xor eax, eax
ret

section .data
scan_fmt db "%u", 0x00
print_fmt db "%u", 0x0A, 0x00