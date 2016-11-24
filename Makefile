CC=@gcc
CFLAGS=-I. -Wno-packed-bitfield-compat
DEPS = iformat.h  jformat.h  mipssim.h  rformat.h fetch.h mipsloader.h datatypes.h issue.h execute.h writeback.h commit.h
OBJ = iformat.o  jformat.o  mipssim.o  rformat.o fetch.o mipsloader.o branch.o commit.o execute.o issue.o loadstore.o writeback.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

mipssim: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f *.o *~ mipssim
