CC = gcc
OBJS = mips_parser.o mips_generator.o mips_text.o mips_util.o runfile.o
CFLAGS = -c
TARGET = runfile

.PHONY: all clean

all: $(TARGET)
default: runfile


runfile.o: main.c mips_parser.h mips_generator.h mips_text.h
	$(CC) $(CFLAGS) -o $@ main.c

mips_parser.o: mips_parser.c mips_parser.h
mips_generator.o: mips_generator.c mips_generator.h
mips_text.o: mips_text.c mips_text.h
mips_util.o: mips_util.c mips_util.h

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS)	

clean:
	rm -f $(OBJS) $(TARGET)

.c.o: 
	$(CC) $(CFLAGS) -o $@ $<
