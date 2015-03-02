#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

size_t reg_nr_to_idx(unsigned nr)
{
	assert(nr >= 5);
	assert(nr <= 8);
	return nr - 5;
}

enum Command {
	CMD_HLT = 0,
	CMD_ADD = 1,
	CMD_SUB = 2,
	CMD_MOV = 3,
	CMD_IR = 4
};

void adjust_reg_8bit(int *reg)
{
	while (*reg < 0) {
		*reg += 0x100;
	}

	while (*reg > 0xFF) {
		*reg -= 0x100;
	}
}

int main()
{
	int halt = 0, error = 0, r;
	int regs[4];
	memset(regs, 0, sizeof(regs));

	while (!halt && !error) {
		enum Command cmd;
		r = scanf("%d", (int *)&cmd);
		if (r != 1) {
			error = 1;
			continue;
		}

		switch (cmd) {
		case CMD_HLT:
			halt = 1;
			break;

		case CMD_ADD:{
				unsigned arg_dest, arg_src;
				r = scanf("%u %u", &arg_dest, &arg_src);
				if (r != 2) {
					error = 1;
					continue;
				}
				regs[reg_nr_to_idx(arg_dest)] +=
				    regs[reg_nr_to_idx(arg_src)];
				adjust_reg_8bit(&regs[reg_nr_to_idx(arg_dest)]);
				break;
			}

		case CMD_SUB:{
				unsigned arg_dest, arg_src;
				r = scanf("%u %u", &arg_dest, &arg_src);
				if (r != 2) {
					error = 1;
					continue;
				}
				regs[reg_nr_to_idx(arg_dest)] -=
				    regs[reg_nr_to_idx(arg_src)];
				adjust_reg_8bit(&regs[reg_nr_to_idx(arg_dest)]);
				break;
			}

		case CMD_MOV:{
				unsigned arg_dest, arg_data;
				r = scanf("%u %u", &arg_dest, &arg_data);
				if (r != 2) {
					error = 1;
					continue;
				}
				regs[reg_nr_to_idx(arg_dest)] = arg_data;
				adjust_reg_8bit(&regs[reg_nr_to_idx(arg_dest)]);
				break;
			}

		case CMD_IR:
			for (size_t i = 0; i < sizeof(regs) / sizeof(*regs);
			     ++i) {
				printf("%u ", regs[i]);
			}
			putchar('\n');
			break;

		default:
			error = 1;
			break;
		}
	}

	return (error == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
