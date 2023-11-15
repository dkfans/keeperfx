CC = $(PREFIX)gcc
AR = $(PREFIX)ar
INCS = -Ideps/luau/Compiler/include -Ideps/luau/VM/include -Ideps/luau/Common/include
CFLAGS = -g -Og -std=c++17 -fmessage-length=0 $(WARNFLAGS) -Werror=implicit $(INCS)
ARFLAGS = rcs


LUAU_VM_FILES = lapi.cpp laux.cpp lbaselib.cpp lbitlib.cpp lbuiltins.cpp lcorolib.cpp ldblib.cpp ldebug.cpp ldo.cpp lfunc.cpp lgc.cpp lgcdebug.cpp linit.cpp lmathlib.cpp lmem.cpp lnumprint.cpp lobject.cpp loslib.cpp lperf.cpp lstate.cpp lstring.cpp lstrlib.cpp ltable.cpp ltablib.cpp ltm.cpp ludata.cpp lutf8lib.cpp lvmexecute.cpp lvmload.cpp lvmutils.cpp
LUAU_COMPILER_FILES = BuiltinFolding.cpp Builtins.cpp BytecodeBuilder.cpp Compiler.cpp ConstantFolding.cpp CostModel.cpp lcode.cpp TableShape.cpp ValueTracking.cpp
OBJSV = $(LUAU_VM_FILES:%.cpp=obj/luauVM/%.o)
OBJSC = $(LUAU_COMPILER_FILES:%.cpp=obj/luauCompiler/%.o)
obj/luauVM/%.o: deps/luau/VM/src/%.cpp
	$(CC) $(CFLAGS) -c -o $@ $<

obj/luauCompiler/%.o: deps/luau/Compiler/src/%.cpp
	$(CC) $(CFLAGS) -c -o $@ $<

obj/luauvm.a: $(OBJSV)
	$(AR) $(ARFLAGS) $@ $(OBJSV)

obj/luaucompiler.a: $(OBJSC)
	$(AR) $(ARFLAGS) $@ $(OBJSC)

clean:
	@$(RM) $(OBJSV) $(OBJSC) obj/luaucompiler.a obj/luauvm.a