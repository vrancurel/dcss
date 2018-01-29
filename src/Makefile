CC = clang++

CFLAGS = \
-std=c++11 -g -Werror -Wall -Wextra -Wpedantic \
-Wdocumentation -Wdocumentation-pedantic \
-Wconversion -Wno-sign-conversion \
-Wunused-exception-parameter -Wunused-macros -Wunused-member-function \
-Wgcc-compat -Wgnu \
-Warray-bounds-pointer-arithmetic \
-Wassign-enum \
-Wbad-function-cast \
-Wconditional-uninitialized \
-Wduplicate-method-arg -Wduplicate-method-match \
-Wfloat-equal \
-Wformat-pedantic \
-Wlarge-by-value-copy \
-Wmissing-noreturn -Wmissing-prototypes \
-Wnested-anon-types \
-Wnon-virtual-dtor \
-Wold-style-cast \
-Woverlength-strings \
-Wshadow \
-Wshift-sign-overflow \
-Wstring-compare -Wstring-conversion -Wstring-plus-char \
-Wsometimes-uninitialized \
-Wswitch-enum \
-Wunreachable-code-aggressive \
-DHAVE_READLINE

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
