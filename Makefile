
CC = g++

CFLAGS = -g -Wall -Werror -DHAVE_READLINE
LDFLAGS = -lcrypto -lreadline

OBJS = \
main.o \
kad_node.o \
kad_routable.o \
kad_file.o \
kad_network.o \
bit_map.o \
shell.o \
cmds.o

kadsim: $(OBJS)
	$(CC) -o kadsim $(OBJS) $(LDFLAGS)

$(OBJS): *.h

%.o: %.cpp
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	$(RM) -f $(OBJS) kadsim
