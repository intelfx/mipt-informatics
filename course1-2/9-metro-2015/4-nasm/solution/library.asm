global subroutine

section .text
subroutine:
mov eax, [esp+4] ; a
mov ecx, [esp+8] ; b
cmp eax, ecx
jg a_plus_b
sub eax, ecx
ret
a_plus_b:
add eax, ecx
ret