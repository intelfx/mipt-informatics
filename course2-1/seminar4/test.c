#include "common.h"

int main()
{
	mkfifo("fifo", 0666);
	int fd = open("fifo", O_RDWR);
	write(fd, "aaa", 3);
	close(fd);
	fd = open("fifo", O_RDONLY|O_NONBLOCK);
	char buf[10];
	write(1, buf, read(fd, buf, 3));
	write(1, "\n", 1);
}
