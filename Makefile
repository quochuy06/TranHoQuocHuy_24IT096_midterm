CC = cc
CPPFLAGS = -D_POSIX_C_SOURCE=200809L -D_DEFAULT_SOURCE -D_NETBSD_SOURCE -D_FILE_OFFSET_BITS=64 -Iinclude
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -O2
LDFLAGS =
LDLIBS =
SOURCES = src/main.c src/options.c src/util.c src/entries.c src/format.c src/listing.c
OBJECTS = src/main.o src/options.o src/util.o src/entries.o src/format.o src/listing.o

.PHONY: all clean
all: myls

myls: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS) $(LDLIBS)

.SUFFIXES:
.SUFFIXES: .c .o

.c.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean:
	rm -f myls $(OBJECTS)
