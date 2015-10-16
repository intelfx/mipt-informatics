extern printf
extern putchar

global main

%define CF_idx 0
%define ZF_idx 6
%define SF_idx 7
%define OF_idx 11

section .text
main:

mov eax, 1
mov ebx, 2
sub eax, ebx

call print_flags

;call set_czso

xor eax, eax
ret


; EFLAGS: flags
; Nothing is clobbered.
print_flags:

pusha

setc al
mov bl, 'C'
call print_flag

setz al
mov bl, 'Z'
call print_flag

sets al
mov bl, 'S'
call print_flag

seto al
mov bl, 'O'
call print_flag

pushf
push 0x0A
call putchar
add esp, 4
popf

popa
ret


; AL: flag value
; BL: flag name
print_flag:

pushf
movzx eax, al
push eax ; flag value
movzx ebx, bl
push ebx ; flag name
push print_flag_fmt
call printf
add esp, 12
popf
ret


set_czso:

call print_flags

mov ecx, CF_idx
call set_flag
call print_flags

mov ecx, ZF_idx
call set_flag
call print_flags

mov ecx, SF_idx
call set_flag
call print_flags

mov ecx, OF_idx
call set_flag
call print_flags

ret


; CL: flag#
set_flag:

pushf
mov ebx, 1
shl ebx, cl
or dword [esp], ebx
popf
ret


; CL: flag#
reset_flag:

pushf
mov ebx, 1
shl ebx, cl
not ebx
and dword [esp], ebx
popf
ret


section .data
print_flag_fmt db '%cF=%u', 0x0A, 0x00