
CC = clang++

CFLAGS = -std=c++11 -g -Wall -Werror -DHAVE_READLINE
LDFLAGS = \
-lcrypto \
-lreadline \
-ljsoncpp \
-lcurl \
-ljsonrpccpp-common \
-ljsonrpccpp-client

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

gethclient.h: geth_spec.json
	jsonrpcstub $< --cpp-client=GethClient

kadclient.h: kad_spec.json
	jsonrpcstub $< --cpp-client=KadClient

$(OBJS): *.h gethclient.h kadclient.h

%.o: %.cpp
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	$(RM) -f $(OBJS) kadsim
