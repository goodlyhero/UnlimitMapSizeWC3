// dllmain.cpp : Определяет точку входа для приложения DLL.
#include "pch.h"
#pragma comment(lib,"Version.lib")
DWORD dwGameDll;
HANDLE thislib;
void Patch(DWORD dwBaseAddress, const char* szData, size_t iSize)
{
    DWORD dwOldProtection = NULL;

    VirtualProtect((LPVOID)dwBaseAddress, iSize, PAGE_EXECUTE_READWRITE, &dwOldProtection);
    CopyMemory((LPVOID)dwBaseAddress, szData, iSize);
    VirtualProtect((LPVOID)dwBaseAddress, iSize, dwOldProtection, NULL);
}

void UnlockMapSize()
{
    DWORD buffer = 0xFFFFFFFF;
    Patch(dwGameDll + 0x6577e4, (char*)&buffer, 4);
    Patch(dwGameDll + 0x66ED7F, (char*)&buffer, 4);
    Patch(dwGameDll + 0x67EC61, (char*)&buffer, 4);
}

DWORD WarcraftVersion()
{
	DWORD dwHandle = NULL;
	DWORD dwLen = GetFileVersionInfoSize(L"Game.dll", &dwHandle);

	LPVOID lpData = new char[dwLen];
	GetFileVersionInfo(L"Game.dll", dwHandle, dwLen, lpData);

	LPBYTE lpBuffer = NULL;
	UINT   uLen = NULL;
	VerQueryValue(lpData,L"\\", (LPVOID*)&lpBuffer, &uLen);

	VS_FIXEDFILEINFO* Version = (VS_FIXEDFILEINFO*)lpBuffer;

	return LOWORD(Version->dwFileVersionLS);
}

DWORD __stdcall FreeThisLibrary(void* d)
{
	FreeLibraryAndExitThread((HMODULE)thislib, 0);
}
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
		thislib = hModule;
		if (GetModuleHandle(L"game.dll") == NULL)
		{
			//Unable to find game.dll
		}
		else if (GetModuleHandle(L"worldedit121.exe") != NULL || GetModuleHandle(L"worldedit.exe") != NULL)
		{
			//Найден модуль редактора, он не должен лежать в адресном пространстве варика.
		}
		else if (WarcraftVersion() != 6401)
		{
			//Version of game.dll is not 1.26a (6401)
		}
		else
		{
			dwGameDll = (DWORD)GetModuleHandle(L"Game.dll");
			UnlockMapSize();
		}
		CreateThread(NULL, 0, &FreeThisLibrary, 0, 0, NULL);
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

