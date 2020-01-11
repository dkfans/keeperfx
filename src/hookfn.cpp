#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <assert.h>
#include <windows.h>
#include <stdint.h>

extern "C" void replaceFn(void* oldFn, void* newFn)
{
	// E9 00000000   jmp rel  displacement relative to next instruction
	unsigned char codeBytes[5] = {0xE9, 0x00, 0x00, 0x00, 0x00};
	uintptr_t p = (uintptr_t)newFn - (uintptr_t)oldFn - sizeof(codeBytes);
	memcpy(&codeBytes[1], &p, sizeof(p));

	SIZE_T bytesWritten = 0;
	BOOL res = WriteProcessMemory(GetCurrentProcess(),
	    oldFn, codeBytes, sizeof(codeBytes), &bytesWritten);
	assert(res);
	assert(bytesWritten == sizeof(codeBytes));
}