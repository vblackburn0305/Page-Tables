CC = clang
CFLAGS = -O2 -g
LDFLAGS = -static

OBJECTS = mlpt.o

all: libmlpt.a

mlpt.o: mlpt.c config.h mlpt.h
	$(CC) $(CFLAGS) -c mlpt.c -o mlpt.o

libmlpt.a: $(OBJECTS)
	ar -rcs libmlpt.a $(OBJECTS)

clean:
	rm -f $(OBJECTS) libmlpt.a
