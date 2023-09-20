CC = $(PREFIX)gcc
AR = $(PREFIX)ar
INCS = -Ideps/enet/include
CFLAGS = -g -Og -std=gnu11 -fmessage-length=0 $(WARNFLAGS) -Werror=implicit $(INCS)
ARFLAGS = rcs

ENET_FILES = callbacks.c compress.c host.c list.c packet.c peer.c protocol.c win32.c
OBJS = $(ENET_FILES:%.c=obj/enet/%.o)

obj/enet/%.o: deps/enet/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

obj/enet.a: $(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)

clean:
	@$(RM) $(OBJS) obj/enet.a