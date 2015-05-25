extern scanf

extern getchar
extern putchar
extern fputs
extern stdout

global main

;
; We'll use scanf for reading radices and the conversion mode (as this is auxiliary).
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

; now zero-pad the output as required.
; first, calculate amount of zero-padding required to make beautiful output
; for input width of K bits.
; range representable with N digits in [out_radix] is equal to [out_radix]^N, thus
; we need to solve [out_radix]^N >= 2^K for minimal integer N.
; The solution is minN = ceil (log_[out_radix] (2^K))
;                      = ceil (log2 (2^K) / log2 ([out_radix]))
;                      = ceil (K / log2 ([out_radix])).

; K...
fild dword [out_pad_to_bits]

; ...log2 ([out_radix])...
fld1
fild dword [out_radix]
fyl2x

; ...K / log2 ([out_radix])...
fdivp

; ...now ceil()...
sub esp, 2
fstcw word [esp]
and word [esp], ~(11 << 10) ; RC field
or word [esp], (10b << 10) ; 10b -- rounding towards +infinity
fldcw word [esp]
add esp, 2
frndint

; ...and finally store the required padding in EDX.
sub esp, 4
fistp dword [esp]
pop edx

; do the padding
.pad_loop:
cmp ecx, edx
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
test ecx, 7 ; 8-1
jnz .no_group_split
; // sunrise by hand, PUSH imm8 zero-extends the immediate...
dec esp
mov byte [esp], ' '
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
cmp ecx, dword [out_pad_to_bits]
jnb .pad_loop_end

; ...still make a group split each 8 bits
test ecx, 7 ; 8-1
jnz .pad_loop_no_group_split
; // sunrise by hand, PUSH imm8 zero-extends the immediate...
dec esp
mov byte [esp], ' '
.pad_loop_no_group_split:

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

; we'll want to terminate bytes of the BCD sequence with spaces rather than newlines
mov byte [terminal_char], ' '

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

;
; first, we need to reconstruct the floating-point value as
; [in_significand] * [in_radix]^(-[in_negative_exponent]).
;
; we have the following primitives:
;  fyl2x: y * log2(x)
;  f2xm1: 2^x - 1 // x lies in [-1; 1]
;  fscale: x * 2^y // y is integer
;
; thus, let S be significand, R be radix and E be exponent, let's rewrite as follows:
;  VALUE = S * R^E
;        = S * 2^(log2(R^E))
;        = S * 2^(E*log2(R))
;        = S * 2^((floor(E*log2(R)) + rem(E*log2(R)))
;        = S * 2^floor(E*log2(R)) * 2^rem(E*log2(R))
;        = S * 2^floor(E*log2(R)) * (2^rem(E*log2(R)) - 1 + 1)

; load negative exponent -E
fild dword [in_negative_exponent]
; get exponent E
fchs
; load radix R
fild dword [in_radix]

; compute ST1 * log2(ST0) = E * log2(R)
fyl2x

; now ST1 = E*log2(R), ST0 = floor(ST1) = floor(E*log2(R))
fld st0
sub esp, 2
fstcw word [esp]
and word [esp], ~(11 << 10) ; RC field
or word [esp], (11b << 10) ; 11b -- rounding towards 0
fldcw word [esp]
add esp, 2
frndint

; now ST0 = E*log2(R), ST1 = floor(E*log2(R))
fxch
; now ST0 = ST0 - ST1 = rem(E*log2(R))
fsub st0, st1

; now ST0 = 2^rem(E*log2(R))
f2xm1
fld1
faddp

; now ST0 = ST0 * 2^ST1 = 2^rem(E*log2(R)) * 2^floor(E*log2(R))
fscale

; now ST0 = S * ST0 = VALUE
fimul dword [in_significand]

; ...and restore the sign
cmp byte [in_sign], 0
jz .no_sign_change
fchs
.no_sign_change:

; now write the outputs
; we force exponent and sign to 0 because we are now printing bare raw representation
mov dword [in_negative_exponent], 0
mov byte [in_sign], 0

; we'll want to zero-pad each written value to a dword boundary
mov dword [out_pad_to_bits], 32

; now write the single precision

; greeting...
push dword [stdout]
push fp_greet_single
call fputs
add esp, 8

; ...and the only dword.
fst dword [in_significand]
call convert_integer

; now write the double precision -- it's a bit more complex.
; we'll have to store the double-precision value in the stack and then
; print high and low dwords separately.
; of course, that would work only if [out_radix] is an exact power of two.

mov eax, dword [out_radix]
dec eax
and eax, dword [out_radix]
jnz normal_exit

; greeting...
push dword [stdout]
push fp_greet_double
call fputs
add esp, 8

sub esp, 8
fst qword [esp]

; ...most significant dword...
mov eax, dword [esp+4]
mov dword [in_significand], eax
mov byte [terminal_char], 0
call convert_integer

; ...least significant dword.
mov eax, dword [esp]
mov dword [in_significand], eax
mov byte [terminal_char], 0x0A
call convert_integer

; finally, the extended precision (80 bit).
; a bit of loop unrolling.

; greeting...
push dword [stdout]
push fp_greet_extended
call fputs
add esp, 8

sub esp, 2 ; two remaining bytes
fstp tword [esp] ; here we pop the value from x87 stack -- we no longer need it, and FST m80 does not exist.

; most significant word... pad it to a word instead.
mov dword [out_pad_to_bits], 16
movzx eax, word [esp+8]
mov dword [in_significand], eax
mov byte [terminal_char], 0
call convert_integer

; ...next to most significant dword... restore dword padding.
mov dword [out_pad_to_bits], 32
mov eax, dword [esp+4]
mov dword [in_significand], eax
; byte [terminal_char] is already 0
call convert_integer

; ...least significant dword.
mov eax, dword [esp]
mov dword [in_significand], eax
mov byte [terminal_char], 0x0A
call convert_integer

; that's all, folks!
add esp, 10
jmp normal_exit

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

movzx eax, byte [terminal_char]
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
fp_greet_single   db "IEEE754 single-precision representation:   ", 0x00
fp_greet_double   db "IEEE754 double-precision representation:   ", 0x00
fp_greet_extended db "IEEE754 extended-precision representation: ", 0x00
scan_fmt_radix db "%u", 0x00
scan_fmt_type db " %c ", 0x00
terminal_char db 0x0A

align 4, db 0

; fixed for now...
bcd_radix dd 10

; default is to pad a byte boundary
out_pad_to_bits dd 8

section .bss align=4

in_radix resd 1
out_radix resd 1
conversion_type resb 1
alignb 4

in_significand resd 1
in_negative_exponent resd 1 ; in fact, this is 'dot position' from least significant digit in the input
in_sign resb 1 ; 1 if negative