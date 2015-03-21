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

mov eax, [esp]
mov ebx, [esp+4]
call gcd
mov ecx, eax

pop eax
pop ebx
mul ebx
div ecx

push eax
push write_fmt
call printf
add esp, 8

xor eax, eax
ret

; input: eax, ebx (unsigned)
; output: eax
gcd:
test ebx, ebx
jz .no_neg ; shortcut
xor edx, edx
div ebx
mov eax, ebx
mov ebx, edx
test ebx, ebx
jnz gcd
test eax, eax
jge .no_neg
neg eax
.no_neg:
ret


section data

read_fmt db "%u %u", 0x00
write_fmt db "%u", 0x0A, 0x00