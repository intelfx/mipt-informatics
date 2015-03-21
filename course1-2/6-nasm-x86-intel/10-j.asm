use32
global main
extern scanf
extern printf
extern putchar
extern malloc
extern free
extern qsort

section text

main:
sub esp, 4
push esp
push read_count_fmt
call scanf
add esp, 8


mov ebx, [esp]
shl dword [esp], 2
call malloc
add esp, 4

push eax ; address
push ebx ; length

push eax
push read_value_fmt
.read_loop:
call scanf
dec ebx
jz .read_loop_end
add dword [esp+4], 4
jmp .read_loop
.read_loop_end:
add esp, 8

mov ebx, [esp] ; length
mov esi, [esp+4] ; address

push predicate
push 4
push ebx
push esi
call qsort
add esp, 16

mov ebx, [esp] ; length
mov esi, [esp+4] ; address
lodsd
push eax
push write_value_fmt
.write_loop:
call printf
dec ebx
jz .write_loop_end
lodsd
mov [esp+4], eax
jmp .write_loop
.write_loop_end:
add esp, 8

push 0x0A
call putchar
add esp, 4

add esp, 4 ; remove length, address on top
call free
add esp, 4

xor eax, eax
ret

; we must be abi compliant here
predicate:
mov ecx, [esp+4] ; first
mov ecx, [ecx]
mov edx, [esp+8] ; second
mov edx, [edx]
xor eax, eax
cmp ecx, edx

mov ecx, 1
mov edx, -1
cmovg eax, ecx
cmovl eax, edx

ret

section data

read_count_fmt db "%u", 0x00
read_value_fmt db "%d", 0x00
write_value_fmt db "%d ", 0x00