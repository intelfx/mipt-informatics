use32
global main
extern puts

section text

main:
push message
call puts
add esp, 4
xor eax, eax
ret

section data

message db "77", 0x0A, 0x00