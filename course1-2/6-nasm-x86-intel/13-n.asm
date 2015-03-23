use32
global main
extern scanf
extern printf
extern putchar
extern malloc
extern free

section .text

main:

mov eax, array_1
call read_array_8

mov eax, array_2
call read_array_8

movq mm0, qword [array_1]
movq mm1, qword [array_2]
paddsb mm0, mm1
movq qword [array_1], mm0

mov esi, array_1
call write_array_8

xor eax, eax
ret

; eax: address
read_array_8:
mov ebx, 8
push eax
push read_value_fmt
.read_loop:
call scanf
dec ebx
jz .read_loop_end
add dword [esp+4], 1
jmp .read_loop
.read_loop_end:
add esp, 8
ret

; esi: address
write_array_8:
mov ebx, 8
lodsb
push eax
push write_value_fmt
.write_loop:
call printf
dec ebx
jz .write_loop_end
lodsb
mov [esp+4], eax
jmp .write_loop
.write_loop_end:
add esp, 8

push 0x0A
call putchar
add esp, 4
ret

section .bss
array_1 resb 8
array_2 resb 8

section .data
read_value_fmt db "%hhu", 0x00
write_value_fmt db "%hhu ", 0x00