global sum1
global sum2

section .text

sum1:

xor eax, eax
xor ecx, ecx
mov edx, dword [esp+4] ; array

.loop:
cmp ecx, dword [esp+8]
jae .loop_end

add eax, [edx+ecx*4]
inc ecx
jmp .loop
.loop_end:

ret


sum2:

fldz
xor ecx, ecx
mov edx, dword [esp+4]

.loop:
cmp ecx, dword [esp+8]
jae .loop_end

fadd qword [edx+ecx*8]
inc ecx
jmp .loop
.loop_end:

ret