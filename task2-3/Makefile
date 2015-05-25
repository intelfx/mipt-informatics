CC = gcc
CFLAGS += -g -Wall -Wextra -std=c99

APP = main

%.o: %.c
	$(CC) -c -m32 $(CFLAGS) $< -o $@

%.o: %.asm
	nasm -f elf32 $< -o $@

%: %.o
	$(CC) -m32 $(LDFLAGS) $^ $(LDLIBS) -o $@

$(APP): main.o util.o

.PHONY: all clean

all: $(APP)

clean:
	rm -vf $(APP) *.o
