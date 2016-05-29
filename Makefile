PROGRAM = iprange_to_hostname
OBJS = ip_to_host.c
SRCS = $(OBJS:%.o=%.c)
CFLAGS = -g -Wall -O2
LDFLAGS = 
CC = gcc


$(PROGRAM):$(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(PROGRAM) $(OBJS) $(LDLIBS)
