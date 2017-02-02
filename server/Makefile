CC := gcc

SHDIR := ../common

OBJS = ftserve.o $(SHDIR)/common.o

all: ftserve

ftserve: $(OBJS)
    @$(cc) -o $@ $(OBJS)

$(OBJS) : %.o: %.c
    @$(cc) -o $@ -c $<

.PHONY:
clean:
    @rm -f *.o ftserve
    @rm -f ../common/*.o
    @echo Done cleaning