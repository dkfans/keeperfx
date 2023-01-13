CC = $(PREFIX)gcc
AR = $(PREFIX)ar
INCS = -Ideps/luau/Compiler/include -Ideps/luau/Compiler/include
CFLAGS = -g -Og -std=gnu11 -fmessage-length=0 $(WARNFLAGS) -Werror=implicit $(INCS)
ARFLAGS = rcs

LUAU_FILES = $(wildcard *.cpp)
OBJS = $(LUAU_FILES:%.c=obj/luau/VM/src/%.o %.c=obj/luau/Compiler/src/%.o)

obj/luau/Compiler/src/%.o: deps/luau/Compiler/src/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

obj/luau/VM/src/%.o: deps/luau/VM/src/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

obj/luau.a: $(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)

clean:
	@$(RM) $(OBJS) obj/luau.a