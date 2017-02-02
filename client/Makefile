CC := gcc
SHDIR := ../common
OBJS = ftclient.o $(SHDIR)/common.o

all: ftclient

ftclient: $(OBJS)
    @$(CC) -o ftclient $(OBJS)

$(OBJS) : %.o: %.common
    @$(CC) -o $@ -c $<

.PHONY:
clean:
    @rm -f *.o ftclient
    @rm -f ../common/*.o
    @echo Done cleaning