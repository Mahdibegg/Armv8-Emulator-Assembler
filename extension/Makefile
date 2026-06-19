CC      ?= gcc
CFLAGS  ?= -std=c17 -g \
	-D_POSIX_SOURCE -D_DEFAULT_SOURCE \
	-Wall -Werror -pedantic \
	-I .

LDLIBS  ?= -lgpiod

.SUFFIXES: .c .o
.PHONY: all clean run

TARGET = led_net_stat

OBJS = \
	led_net_stat.o \
	network_status.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDLIBS)

%.o: %.c network_status.h
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	sudo ./$(TARGET)

clean:
	$(RM) $(TARGET) $(OBJS)