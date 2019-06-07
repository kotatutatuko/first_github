9cc: 9cc.c

test: 9cc
		./test.sh
		./9cc -test


clean:
		rm -f 9cc *.o *~ tmp* a.out

gdb:
		gcc -g 9cc.c
		gdb a.out
