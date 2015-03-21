"mov rdx, 1\n" /* factorial */
"mov rbx, 1\n" /* candidate */
"factorial_loop:\n"
"cmp rdx, rax\n"
"je factorial_end\n"
"inc rbx\n"
"imul rdx, rbx\n"
"jmp factorial_loop\n"
"factorial_end:\n"