global main
extern fread
extern fwrite
extern stdin
extern stdout
extern puts
extern putchar
extern printf

extern strlen

extern islower
extern isupper
extern isdigit
extern isalpha
extern isalnum

extern toupper
extern tolower

%define MAX_TEXT_SIZE 100

section .text

main:

; read input into the buffer
push dword [stdin]
push MAX_TEXT_SIZE
push 1
push input
call fread
add esp, 16

; terminate with null byte (just in case)
mov byte [input+eax], 0

; get input size, it may not coincide with amount of data read (e. g. text, 0, garbage)
push input
call strlen
add esp, 4
mov [input_size], eax

; prepare variables for the rule, while at it
mov dword [output_ptr], input
mov dword [output_size], eax

; write back the input
push dword [stdout]
push eax
push 1
push input
call fwrite
add esp, 16

push 0x0A
call putchar
add esp, 4

; check for the property
call dword [property_ptr]

; execute the rule
mov eax, [rule_1_ptr]
mov ebx, [rule_2_ptr]
cmovnc eax, ebx
call eax

; write back the rule
push eax
push rule_fmt
call printf
add esp, 8

; write back the output
push dword [stdout]
push dword [output_size]
push 1
push dword [output_ptr]
call fwrite
add esp, 16

push 0x0A
call putchar
add esp, 4

normal_exit:

mov eax, 0
ret

error_exit:

mov eax, 1
ret

true_ret:

stc
ret

false_ret:

clc
ret

;
; All subroutines follow a custom abi wrt. saved/clobbered registers
; * EBP, ESP - saved
; * everything else - clobbered
; and a custom abi wrt. parameter passing
; * parameters are passed in clobbered registers in order
; * integer return values are in EAX
; * boolean return values are in CF
;


property_1:

mov ebx, input
add ebx, dword [input_size]
dec ebx

; check for uppercase letter
movzx eax, byte [ebx]
push eax
call isupper
add esp, 4
test eax, eax
jz false_ret

; check for other occurrences
mov esi, input
mov ecx, [input_size]
dec ecx
mov al, byte [ebx]
repne scasb
je false_ret

jmp true_ret


property_2:

; check for digit at the beginning
movzx eax, byte [input]
push eax
call isdigit
add esp, 4
test eax, eax
jz false_ret

; check for digit at the end
mov eax, input
add eax, dword [input_size]
dec eax
movzx eax, byte [eax]

; ...btw, check for equality...
cmp al, byte [input]
je false_ret

push eax
call isdigit
add esp, 4
test eax, eax
jz false_ret

jmp true_ret


property_3:

; check for letter at the beginning
movzx eax, byte [input]
push eax
call isalpha
add esp, 4
test eax, eax
jz false_ret

; check for letter at the end
mov eax, input
add eax, dword [input_size]
dec eax
movzx eax, byte [eax]
push eax
call isalpha
add esp, 4
test eax, eax
jz false_ret

jmp true_ret


property_4:

xor ebx, ebx
mov esi, input

.check_loop:
xor eax, eax
lodsb

; if we're at the end and still in the loop, we've not reached the objective
test eax, eax
jz false_ret

push eax
call isalpha
add esp, 4
test eax, eax
jz .check_loop
inc ebx
cmp ebx, 3
jb .check_loop

jmp true_ret


property_5:

xor ebx, ebx
mov esi, input

.check_loop:
xor eax, eax
lodsb

test eax, eax
jz .check_loop_end

push eax

call isupper
test eax, eax
jz .no_upper
inc ebx
.no_upper:

call islower
test eax, eax
jz .no_lower
dec ebx
.no_lower:

add esp, 4

jmp .check_loop

.check_loop_end:

test ebx, ebx
jz true_ret
jmp false_ret


property_6:

mov esi, input

.check_loop:
xor eax, eax
lodsb

; if we've reached the end and still in the loop, we've reached the objective
test eax, eax
jz true_ret

push eax
call isalnum
add esp, 4
test eax, eax
jz false_ret

jmp .check_loop


rule_1_0:

mov eax, rule_1_0_desc
ret


rule_2_0:

mov eax, rule_2_0_desc
ret


rule_1_1:

mov esi, input

.processing_loop:
xor eax, eax
lodsb

test eax, eax
jz .exit

mov ebx, eax

push eax
call isupper
add esp, 4
test eax, eax
jz .skip

inc bl
cmp bl, 'Z'
mov cl, 'A'
cmova ebx, ecx
mov byte [esi-1], bl

.skip:
jmp .processing_loop

.exit:
mov eax, rule_1_1_desc
ret


rule_1_2:

mov esi, input

.processing_loop:
xor eax, eax
lodsb

test eax, eax
jz .exit

mov ebx, eax

push eax
call isdigit
add esp, 4
test eax, eax
jz .skip
cmp bl, '0'
je .skip

add bl, 'a' - '1'
mov byte [esi-1], bl

.skip:
jmp .processing_loop

.exit:
mov eax, rule_1_2_desc
ret


rule_1_3:

mov esi, input

.processing_loop:
xor eax, eax
lodsb

test eax, eax
jz .exit

mov ebx, eax

push eax
call isupper
add esp, 4
test eax, eax
jz .skip

sub bl, 'A' - 1

movzx ax, bl
mov bl, 10
div bl

add ah, '0'
mov byte [esi-1], ah

.skip:
jmp .processing_loop

.exit:
mov eax, rule_1_3_desc
ret


rule_1_4:

mov esi, input

.processing_loop:
xor eax, eax
lodsb

test eax, eax
jz .exit

push eax
call toupper
add esp, 4
mov byte [esi-1], al

jmp .processing_loop

.exit:
mov eax, rule_1_4_desc
ret


rule_1_5:

mov esi, input

.processing_loop:
xor eax, eax
lodsb

test eax, eax
jz .exit

push eax
call tolower
add esp, 4
mov byte [esi-1], al

jmp .processing_loop

.exit:
mov eax, rule_1_5_desc
ret


rule_1_6:

mov esi, input

.processing_loop:
xor eax, eax
lodsb

test eax, eax
jz .exit

mov ebx, eax

push eax
call isupper
add esp, 4
test eax, eax
jz .skip

sub bl, 'A'
mov al, 'Z'
sub al, bl

mov byte [esi-1], al

.skip:
jmp .processing_loop

.exit:
mov eax, rule_1_3_desc
ret


rule_2_1:

mov esi, input
mov edi, output

.processing_loop_1:
xor eax, eax
lodsb

test eax, eax
jz .processing_loop_1_end

mov ebx, eax

push eax
call isdigit
add esp, 4
test eax, eax
jz .skip_1

mov eax, ebx
stosb

.skip_1:
jmp .processing_loop_1
.processing_loop_1_end:

mov esi, input

.processing_loop_2:
xor eax, eax
lodsb

test eax, eax
jz .processing_loop_2_end

mov ebx, eax

push eax
call isdigit
add esp, 4
test eax, eax
jnz .skip_2

mov eax, ebx
stosb

.skip_2:
jmp .processing_loop_2
.processing_loop_2_end:

.exit:
mov dword [output_ptr], output
mov eax, rule_2_1_desc
ret


rule_2_2:

mov esi, input
mov edi, input
add edi, dword [input_size]
dec edi

.processing_loop:
xor eax, eax

mov al, byte [esi]
xchg al, byte [edi]
mov byte [esi], al

inc esi
dec edi

cmp esi, edi
jb .processing_loop

.exit:
mov eax, rule_2_2_desc
ret


rule_2_3:

mov esi, input
mov edi, output

.processing_loop:
xor eax, eax
lodsb

test eax, eax
jz .exit

stosb
stosb

jmp .processing_loop

.exit:
mov dword [output_ptr], output
shl dword [output_size], 1
mov eax, rule_2_3_desc
ret


rule_2_4:

mov esi, input
mov edi, input
movzx ebx, byte [input]

test ebx, ebx
jz .exit

inc esi
inc edi

.processing_loop:
xor eax, eax
lodsb

test eax, eax
jz .exit

cmp eax, ebx
je .skip

stosb

.skip:
jmp .processing_loop

.exit:
sub edi, input
mov dword [output_size], edi
mov eax, rule_2_4_desc
ret


rule_2_5:

sub esp, 0x100 ; reserve space for the character count table

mov edi, esp
mov ecx, 0x100 / 4
xor eax, eax
rep stosd ; clear the character count table

mov ebx, esp
mov esi, input

.counting_loop:
xor eax, eax
lodsb

test eax, eax
jz .counting_loop_end

inc byte [ebx+eax]

jmp .counting_loop
.counting_loop_end:

mov esi, input
mov edi, input

.processing_loop:
xor eax, eax
lodsb

test eax, eax
jz .exit

cmp byte [ebx+eax], 1
ja .skip

stosb

.skip:
jmp .processing_loop

.exit:
sub edi, input
mov dword [output_size], edi
mov eax, rule_2_5_desc
add esp, 0x100 ; deallocate the space of the character count table
ret


rule_2_6:

mov esi, input
mov edi, input
xor ebx, ebx

.processing_loop:
xor eax, eax
lodsb

test eax, eax
jz .exit

cmp eax, ebx
je .skip

stosb

.skip:
mov ebx, eax
jmp .processing_loop

.exit:
sub edi, input
mov dword [output_size], edi
mov eax, rule_2_6_desc
ret


section .bss

input_size resd 1
output_size resd 1
output_ptr resd 1
input resb MAX_TEXT_SIZE + 1
output resb MAX_TEXT_SIZE * 2 + 1 ; for rule 2.3


section .data

rule_fmt db "Rule applied: '%s'.", 0x0A, 0

rule_1_0_desc db "The property is fulfilled", 0
rule_1_1_desc db "Each upper-case letter is replaced with the next one", 0
rule_1_2_desc db "Each non-zero digit is replaced with a lower-case letter of a corresponding ordinal", 0
rule_1_3_desc db "Each upper-case letter is replaced with a digit equal to the ordinal of the letter mod 10", 0
rule_1_4_desc db "Each lower-case letter is replaced with a corresponding upper-case letter", 0
rule_1_5_desc db "Each upper-case letter is replaced with a corresponding lower-case letter", 0
rule_1_6_desc db "Each upper-case letter of ordinal I is replaced with an upper-case letter of ordinal ord(Z)+1-I", 0

rule_2_0_desc db "The property is NOT fulfilled", 0
rule_2_1_desc db "All digits in text are moved into its beginning, with order preserved", 0
rule_2_2_desc db "The text is reversed in-place", 0
rule_2_3_desc db "Each character of the text is duplicated", 0
rule_2_4_desc db "All non-first occurrences of the first character in the text are removed", 0
rule_2_5_desc db "All characters with multiple occurrences are removed from the text", 0
rule_2_6_desc db "All characters equal to the previous one are removed", 0

property_ptr dd property_1
rule_1_ptr dd rule_1_1
rule_2_ptr dd rule_2_1