extern scanf

extern getchar
extern putchar
extern fputs
extern stdout

global main

;
; We'll use printf for printing greeting messages and reading radices
; (as this is auxiliary).
;

section .text
main:

; read input radix
push dword [stdout]
push greet_in_radix
call fputs
add esp, 8

push in_radix
push scan_fmt_radix
call scanf
add esp, 8

; read output radix
push dword [stdout]
push greet_out_radix
call fputs
add esp, 8

push out_radix
push scan_fmt_radix
call scanf
add esp, 8

; read conversion type
push dword [stdout]
push greet_type
call fputs
add esp, 8

push conversion_type
push scan_fmt_type
call scanf
add esp, 8

; read the input, it is common for all conversion types
call read_input

; dispatch based on the value of flag
cmp byte [conversion_type], 'i'
je convert_integer

cmp byte [conversion_type], 'b'
je convert_bcd

cmp byte [conversion_type], 'p'
je convert_bcd ; the same function -- they share a lot of code

cmp byte [conversion_type], 'f'
je convert_fp

jmp error_exit

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


convert_integer:

; fractional part is not allowed in the integer mode
cmp dword [in_negative_exponent], 0
jne error_exit

; load significand
mov eax, dword [in_significand]

; since we generate output digits in reverse order (from least to most significant),
; we'll write them to our stack frame, getting a big-endian character sequence.
push ebp
mov ebp, esp

; // sunrise by hand, PUSH r8 does not exist...
dec esp
mov byte [esp], 0

; now dispatch to writing two's complement if out_radix is 2
cmp dword [out_radix], 2
je convert_integer_twos_complement

; otherwise continue with normal implementation

xor ecx, ecx

.loop:

test eax, eax
jz .loop_end

; divide significand by output radix, remainder will be the next digit
xor edx, edx
div dword [out_radix]

; make it a character
add dl, '0'

; check if it should've been a letter
cmp dl, '9'
jna .output_ok
add dl, 'A' - ('0' + 10)
.output_ok:

; store the character and go for the next one
; // sunrise by hand, PUSH r8 does not exist...
dec esp
mov byte [esp], dl
inc ecx
jmp .loop

.loop_end:

; zero-pad the output as required
.pad_loop:
cmp ecx, dword [out_padding]
jnb .pad_loop_end
; // sunrise by hand, PUSH imm8 zero-extends the immediate...
dec esp
mov byte [esp], '0'
inc ecx
jmp .pad_loop
.pad_loop_end:

; finally, write the minus sign if required
cmp byte [in_sign], 0
je .no_negate
; // sunrise by hand, PUSH imm8 zero-extends the immediate...
dec esp
mov byte [esp], '-'
.no_negate:

; write the output
jmp write_output


convert_integer_twos_complement:

; negate significand if required
cmp byte [in_sign], 0
je .no_negate
neg eax
.no_negate:

xor edx, edx
xor ecx, ecx

.loop:

test eax, eax
jz .loop_end

; make a group split each 8 bits
cmp ecx, 8
jb .no_group_split
; // sunrise by hand, PUSH imm8 zero-extends the immediate...
dec esp
mov byte [esp], ' '
xor ecx, ecx
.no_group_split:

; divide significand by output radix, remainder will be the next digit
shr eax, 1
setc dl

; make it a character
add dl, '0'

; store the character and go for the next one
; // sunrise by hand, PUSH r8 does not exist...
dec esp
mov byte [esp], dl
inc ecx
jmp .loop

.loop_end:

; zero-pad the last byte
.pad_loop:
cmp ecx, 8
jnb .pad_loop_end
; // sunrise by hand, PUSH imm8 zero-extends the immediate...
dec esp
mov byte [esp], '0'
inc ecx
jmp .pad_loop
.pad_loop_end:

; write the output
jmp write_output


convert_bcd:

; fractional part is not allowed in the bcd mode
cmp dword [in_negative_exponent], 0
jne error_exit

; load significand
mov eax, dword [in_significand]

; here we'll use the same idea as in convert_integer.
;
; since we generate BCD bytes in reverse order (from least to most significant),
; we'll write them to our stack frame, getting a big-endian BCD sequence.

push ebp
mov ebp, esp

xor ecx, ecx

.loop:

test eax, eax
jz .loop_end

; divide significand by intermediate (BCD) radix, remainder will be the next digit
xor edx, edx
div dword [bcd_radix]

; store the character and go for the next one
; if we're in packed mode, let's alternate between starting a new byte
; and ORing the character into the most significant tetrade of the existing byte.
cmp byte [conversion_type], 'p'
jne .bcd_new_byte ; not packed? let's start a new byte.
test cl, 1
jz .bcd_new_byte ; current digit is even? let's start a new byte.

shl dl, 4
or byte [esp], dl
jmp .bcd_written

.bcd_new_byte:
; // sunrise by hand, PUSH r8 does not exist...
dec esp
mov byte [esp], dl

.bcd_written:
inc ecx
jmp .loop
.loop_end:

; now output the BCD chain [ESP; EBP) byte-by-byte

; first, calculate amount of zero-padding required to make beautiful output.
; range representable with N digits in [out_radix] is equal to [out_radix]^N, thus
; we need to solve [out_radix]^N >= 256 for minimal integer N.
; The solution is minN = ceil (log_[out_radix] (256))
;                      = ceil (log2 (256) / log2 ([out_radix]))
;                      = ceil (8 / log2 ([out_radix])).

; 8...
push 8
fild dword [esp]

; ...log2 ([out_radix])...
fld1
fild dword [out_radix]
fyl2x

; ...8 / log2 ([out_radix])...
fdivp

; ...now ceil()...
fstcw word [esp]
and word [esp], ~(11 << 10) ; RC field
or word [esp], (10b << 10) ; 10b -- rounding towards +infinity
fldcw word [esp]
frndint

; ...and finally store the padding.
fistp dword [esp]
pop dword [out_padding]

; we'll want to terminate bytes of the BCD sequence with spaces rather than newlines
mov dword [terminal_char], ' '

.bcd_write_loop:

cmp esp, ebp
jnb .bcd_write_loop_end

; // sunrise by hand, POP r8 does not exist...
movzx eax, byte [esp]
inc esp
mov dword [in_significand], eax
; dword [in_negative_exponent] is already 0
call convert_integer

; the remaining bytes should never be printed with a leading minus sign
mov byte [in_sign], 0

jmp .bcd_write_loop
.bcd_write_loop_end:

; print a final newline
push 0x0A
call putchar
add esp, 4

mov esp, ebp
pop ebp
jmp normal_exit


convert_fp:


read_input:

mov dword [in_significand], 0
mov dword [in_negative_exponent], 0

; read char-by-char and keep state in EBX between invocations of getchar
xor ebx, ebx

; read minus sign, if any
call getchar
xor ecx, ecx
cmp al, '-'
sete cl
mov byte [in_sign], cl

; if we've read whatever is not a minus sign (then it must be a digit), skip doing another getchar()
jne .loop_skip_getchar

.loop:
call getchar
.loop_skip_getchar:

; end of input?
cmp al, 0x0A
je true_ret

; dot?
cmp al, '.'
jne .input_no_dot

test ebx, 0x1
jnz error_exit ; duplicate dot
or ebx, 0x1
jmp .loop

.input_no_dot:

; digit?
cmp al, '0'
jb .input_no_digit
cmp al, '9'
ja .input_no_digit

sub al, '0'
jmp .input_parsed

.input_no_digit:

; letter?
cmp al, 'a'
jb .input_no_small_letter
cmp al, 'z'
ja .input_no_small_letter

sub al, 'a' - 10
jmp .input_parsed

.input_no_small_letter:

; letter?
cmp al, 'A'
jb .input_no_cap_letter
cmp al, 'Z'
ja .input_no_cap_letter

sub al, 'A' - 10
jmp .input_parsed

.input_no_cap_letter:

; wrong input then
jmp error_exit

.input_parsed:

movzx ecx, al
; check if this digit is allowed in our radix
cmp ecx, dword [in_radix]
jae error_exit

; significand = significand * radix + digit
mov eax, dword [in_significand]
mul dword [in_radix]
add eax, ecx
mov dword [in_significand], eax

; increment our "dot position" if we've just read a digit after the dot
test ebx, 0x1
jz .still_integer_part
inc dword [in_negative_exponent]
.still_integer_part:

jmp .loop


write_output:

mov eax, esp

push dword [stdout]
push eax
call fputs
add esp, 8

mov eax, dword [terminal_char]
test eax, eax
jz .no_terminal_char

push eax
call putchar
add esp, 4

.no_terminal_char:

mov esp, ebp
pop ebp
jmp normal_exit


section .data

greet_in_radix db "Enter input radix: ", 0x00
greet_out_radix db "Enter output radix: ", 0x00
greet_type db "Say type of conversion [i(nteger), f(p), b(cd), p(acked_bcd)]: ", 0x00
greet_data db "Enter data: ", 0x00
scan_fmt_radix db "%u", 0x00
scan_fmt_type db " %c ", 0x00

terminal_char dd 0x0A

; fixed for now...
bcd_radix dd 10

section .bss align=4

in_radix resd 1
out_radix resd 1
out_padding resd 1
conversion_type resb 1
resd 3 ; align

in_significand resd 1
in_negative_exponent resd 1 ; in fact, this is 'dot position' from least significant digit in the input
in_sign resb 1 ; 1 if negative