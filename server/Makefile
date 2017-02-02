CC := gcc

SHDIR := ../common

OBJS = ftserve.o $(SHDIR)/common.o

all: ftserve

ftserve: $(OBJS)
    @$(CC) -o $@ $(OBJS)

$(OBJS) : %.o: %.c
    @$(CC) -o $@ -c $<

.PHONY:
clean:
    @rm -f *.o ftserve
    @rm -f ../common/*.o
    @echo Done cleaning