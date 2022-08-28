// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <cstdint>
#include <Windows.h>
#include <cstddef>
#include <process.h>

DWORD dwSAMPAddr = NULL;

void* memcpy_safe(void* _dest, const void* _src, size_t stLen)
{
	if (_dest == nullptr || _src == nullptr || stLen == 0)
		return nullptr;

	MEMORY_BASIC_INFORMATION	mbi;
	VirtualQuery(_dest, &mbi, sizeof(mbi));
	VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READWRITE, &mbi.Protect);

	void* pvRetn = memcpy(_dest, _src, stLen);
	VirtualProtect(mbi.BaseAddress, mbi.RegionSize, mbi.Protect, &mbi.Protect);
	FlushInstructionCache(GetCurrentProcess(), _dest, stLen);
	return pvRetn;
}
int memset_safe(void* _dest, int c, uint32_t len)
{
	uint8_t* dest = (uint8_t*)_dest;
	uint8_t buf[4096];

	memset(buf, c, (len > 4096) ? 4096 : len);

	for (;;)
	{
		if (len > 4096)
		{
			if (!memcpy_safe(dest, buf, 4096))
				return 0;
			dest += 4096;
			len -= 4096;
		}
		else
		{
			if (!memcpy_safe(dest, buf, len))
				return 0;
			break;
		}
	}

	return 1;
}


void mainThread(void* pvParams)
{
	dwSAMPAddr = (DWORD)GetModuleHandleA("samp.dll");


	memcpy_safe((uint8_t*)dwSAMPAddr + 0x63DA0, (BYTE*)"\x90\x90\x90\x90", 4);
	
}
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
		_beginthread(mainThread, NULL, NULL);
		break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

