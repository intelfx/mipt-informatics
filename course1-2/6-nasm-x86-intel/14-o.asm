use32
global _start

section .text

_start:
sub esp, 4

mov eax, 3
mov ebx, 0
mov ecx, esp
mov edx, 1
int 0x80

or byte [esp], 111b

mov eax, 4
mov ebx, 1
mov ecx, esp
mov edx, 1
int 0x80

add esp, 4
mov eax, 1
xor ebx, ebx
int 0x80