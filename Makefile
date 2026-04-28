njr-bash: Shell.o func.o
	gcc Shell.o func.o -o njr-bash
Shell.o: Shell.c func.h
	gcc -c Shell.c
func.o: func.c func.h
	gcc -c func.c
clean:
	rm -f *.o njr-bash