use32
global main
extern getchar
extern printf

section .text

main:
pusha

call getchar

cmp al, 'n'
mov edx, test_nop
je .end_input

cmp al, 'f'
mov edx, test_fdiv
je .end_input

cmp al, 'i'
mov edx, test_idiv
je .end_input

mov eax, 1
ret

.end_input:


call edx
; time in eax
shr eax, 16

push eax
push print_fmt
call printf
add esp, 8

popa
xor eax, eax
ret

%macro rdtsc_test_prologue 0
lfence
rdtsc
mov esi, eax
%endmacro

%macro rdtsc_test_epilogue 0
lfence
rdtsc
mov edi, eax

lfence
rdtsc

sub eax, edi
sub edi, esi
add eax, edi
ret
%endmacro

test_nop:
rdtsc_test_prologue
%rep 65536
nop
%endrep
rdtsc_test_epilogue

test_idiv:
rdtsc_test_prologue
%rep 65536
mov eax, 100500
cdq
mov ebx, 1
idiv ebx
%endrep
rdtsc_test_epilogue

test_fdiv:
rdtsc_test_prologue
%rep 65536
fld dword [dividend]
fdiv dword [divisor]
%endrep
rdtsc_test_epilogue

section .data
dividend dd 100.5
divisor dd 4.2

total_time dd 0
test_ptr dd 0
print_fmt db "%u", 0x0A, 0x00