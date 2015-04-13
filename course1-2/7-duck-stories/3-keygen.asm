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
sub eax, 100500

push eax
push print_fmt
call printf
add esp, 8

xor eax, eax
ret

section .data
scan_fmt db "%u", 0x00
print_fmt db "%d", 0x0A, 0x00