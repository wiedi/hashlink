SRCS=hashlink.c
OBJS=$(SRCS:.c=.o)
CFLAGS?=-g -Wall -Wextra -O3 -std=c99

all: $(OBJS)
	$(CC) -o hashlink $(OBJS)

.c.o:
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f hashlink *.o
