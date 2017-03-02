CC=gcc
DEPS=hidapi.h smpl1.h gpio.h
OBJ=gpio.o hid.o scan-c.o smpl1.o
LIBS=-lusb-1.0

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

test1: $(OBJ)
	gcc -o $@ $^ $(LIBS)

.PHONY: clean
clean:
	rm -f *.o *~ test1
