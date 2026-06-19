CC      ?= gcc
CFLAGS  ?= -std=c17 -g \
	-D_POSIX_SOURCE -D_DEFAULT_SOURCE \
	-Wall -Werror -pedantic \
	-I .

.PHONY: all clean

all: led_net_stat

LED_NET_STAT_OBJS = \
	led_net_stat.o \
	network_status.o

led_net_stat: $(LED_NET_STAT_OBJS)
	$(CC) $(CFLAGS) -o led_net_stat $(LED_NET_STAT_OBJS) -lgpiod

clean:
	$(RM) led_net_stat *.o