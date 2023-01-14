CC = $(PREFIX)gcc
AR = $(PREFIX)ar
INCS = -Ideps/luau/Compiler/include -Ideps/luau/VM/include -Ideps/luau/Common/include
CFLAGS = -g -Og -fmessage-length=0 $(WARNFLAGS) -Werror=implicit $(INCS)
ARFLAGS = rcs


LUAU_VM_FILES = lapi.cpp laux.cpp lbaselib.cpp lbitlib.cpp lbuiltins.cpp lcorolib.cpp ldblib.cpp ldebug.cpp ldo.cpp lfunc.cpp lgc.cpp lgcdebug.cpp linit.cpp lmathlib.cpp lmem.cpp lnumprint.cpp lobject.cpp loslib.cpp lperf.cpp lstate.cpp lstring.cpp lstrlib.cpp ltable.cpp ltablib.cpp ltm.cpp ludata.cpp lutf8lib.cpp lvmexecute.cpp lvmload.cpp lvmutils.cpp
LUAU_COMPILER_FILES = BuiltinFolding.cpp Builtins.cpp BytecodeBuilder.cpp Compiler.cpp ConstantFolding.cpp CostModel.cpp lcode.cpp TableShape.cpp ValueTracking.cpp
OBJS = $(LUAU_VM_FILES:%.cpp=obj/luau/VM/src/%.o) $(LUAU_COMPILER_FILES:%.cpp=obj/luau/Compiler/src/%.o)

obj/luau/Compiler/src/%.o: deps/luau/Compiler/src/%.cpp
	$(CC) $(CFLAGS) -c -o $@ $<

obj/luau/VM/src/%.o: deps/luau/VM/src/%.cpp
	$(CC) $(CFLAGS) -c -o $@ $<

obj/luau.a: $(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)

clean:
	@$(RM) $(OBJS) obj/luau.a