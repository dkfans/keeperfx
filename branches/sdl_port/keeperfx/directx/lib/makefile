# you should install the DirectX headers before running this!

CC =gcc

lib%.a: %.def %.o
	dlltool -k --output-lib $@ --def $*.def
	ar r $@ $*.o
	ranlib $@

lib%.a: %.def
	dlltool -k --output-lib $@ --def $<

all : libd3d8.a libd3dx8d.a libd3dxof.a libddraw.a libdinput.a libdinput8.a libdplayx.a \
libdpnaddr.a libdpnet.a libdpnlobby.a libdpvoice.a libdsound.a libdsetup.a libdxguid.a

libdxguid.a : dxguid.c
	gcc -c -Wall dxguid.c
	ar -r libdxguid.a dxguid.o
	ranlib libdxguid.a
	@echo move the created libraries to the lib dir.
	@echo happy coding !!

libdinput8.a : dinput8.def dinput.o
	dlltool -k --output-lib libdinput8.a --def dinput8.def
	ar r libdinput8.a dinput.o
	ranlib libdinput8.a

