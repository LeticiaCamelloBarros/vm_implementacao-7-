CC      = gcc
CFLAGS  = -Wall -Wextra -std=c99 -g -D_POSIX_C_SOURCE=200809L
LDFLAGS = -pthread
LDLIBS  =
TARGET  = vm
SRCS    = main.c

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET) $(LDFLAGS) $(LDLIBS)

clean:
	rm -f $(TARGET) *.o correct.txt evidencias.log

run: $(TARGET)
	./$(TARGET) address.txt lru fifo

.PHONY: all clean run
