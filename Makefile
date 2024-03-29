CFLAGS=-Wall -std=c11
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

9cc: $(OBJS)
		$(CC) -o 9cc $(OBJS) $(LDFLAGS)

$(OBJS): 9cc.h

test: 9cc
		./9cc -test
		./test.sh


clean:
		rm -f 9cc *.o *~ tmp* a.out

gdb: $(OBJS)
		$(CC) $(OBJS) $(LDFLAGS) -g
		gdb a.out
