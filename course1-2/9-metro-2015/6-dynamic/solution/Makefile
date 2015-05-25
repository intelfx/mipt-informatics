liblibrary.so: library.asm
	nasm -f elf32 library.asm -o library.o
	gcc -m32 library.o -shared -o liblibrary.so

clean:
	rm -f library.o liblibrary.so
