CC      = gcc
CFLAGS  = -Wall -Wextra -std=c99 -g -D_POSIX_C_SOURCE=200809L
LDFLAGS = -pthread
LDLIBS  =
TARGET  = vm
SRCS    = main.c busca.c algo.c TLB_ops.c MF_PD.c tradu_regis.c
HDRS    = busca.h algo.h TLB_ops.h MF_PD.h tradu_regis.h

all: $(TARGET)

$(TARGET): $(SRCS) $(HDRS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET) $(LDFLAGS) $(LDLIBS)

clean:
	rm -f $(TARGET) *.o correct.txt evidencias.log

run: $(TARGET)
	./$(TARGET) address.txt lru fifo

.PHONY: all clean run
