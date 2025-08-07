/*----------------------------------------------------------------------------*/
/*                                                                            */
/*    Copyright (c) Aaron Wilk 2025, All rights reserved.                     */
/*                                                                            */
/*    Module:     Hook.h			                                          */
/*    Author:     Aaron Wilk                                                  */
/*    Created:    25 June 2025                                                */
/*                                                                            */
/*    Revisions:  V0.1                                                        */
/*                                                                            */
/*----------------------------------------------------------------------------*/

#pragma once
#include <exception>
#include <cstring>
#include <vector>
#include <MinHook.h>
#include <libloaderapi.h>
#include <Psapi.h>
#include <cassert>
#include <utility>
#include <iostream>

#define BASE ((unsigned long long)GetModuleHandle(nullptr))

//Defines a hooking objects and it parameters. Does NOT register the hook with MinHook
#define HOOK(name, addr, lamb, sig) \
Hook<sig>* name##(); \
inline Hook<sig>* H_##name = new Hook<sig>(addr, lamb, false); \
inline Hook<sig>* name##() { return H_##name; } \

//Registers the hook with MinHook
#define HOOK_INIT(ptr) ptr->Create()
//Destroy the hook object to prevent memory leeks
#define HOOK_DESTROY(ptr) ptr->Disable(); DestroyHookInternal(ptr)
#define HOOK_ENABLE(ptr) ptr->Enable()
#define HOOK_DISABLE(ptr) ptr->Disable()
#define HOOK_IS_INIT(ptr) ptr->IsInitialized()
#define HOOK_CALL_ORIGINAL(ptr, ...) ptr->CallOriginalFunction(__VA_ARGS__)

enum SearchType
{
	FAST = 0,
	SAFE = 1,
	ALL = 2
};

template <typename>
class Hook;

template <typename Ret, typename... Args>
class Hook<Ret(Args...)>
{
public:
    Hook(const char* pat, const char* mak, Ret(__fastcall* hookFunc)(Args...), SearchType fsearch = FAST, bool bRunInit = true); //pattern and mask, use the \x00 format on sigs, x and ? on masks (? is wildcard)
    Hook(unsigned long long addr, Ret(__fastcall* hookFunc)(Args...), bool bRunInit = true);  //The address of the function.
	Hook(Ret(__fastcall* ptr)(Args...), Ret(__fastcall* hookFunc)(Args...), bool bRunInit = true); //Use reinterpret_cast<Ret(__fastcall*)(Args...)>(vtable[index]) when inputing a vtable entry
    ~Hook();

private:
    bool enabled;
    bool initialized;
	SearchType Stype;

	unsigned long long FunctionPointer;
    const char* pattern;
    const char* mask;
    using Function_t = Ret(__fastcall*)(Args...);
    bool Init();

public:
	Function_t OriginalFunction;

protected:
	Function_t hookedFunction;

public:
	void Create();
    void Enable();
    void Disable();

	Ret CallOriginalFunction(Args... args);

	bool IsInitialized();
	bool IsEnabled();

protected:
	static unsigned long long FindPattern(const char* pattern, const char* mask, unsigned long long base, unsigned __int64 size);
	static unsigned long long FindPatternS(const char* pattern, const char* mask, unsigned long long base, unsigned __int64 size);
	static unsigned long long FindPatternAll(const char* signature, const char* mask);
	static unsigned long long GetModuleBase();
	static unsigned long long GetModuleSize();
	static bool GetTextSection(unsigned long long& textBase, unsigned __int64& textSize);

};

template<typename Ret, typename ...Args>
Hook<Ret(Args...)>::Hook(const char* pat, const char* mak, Ret(__fastcall* hookFunc)(Args...), SearchType stype, bool bRunInit)
{
	pattern = pat;
	mask = mak;
	enabled = false;
	initialized = false;
	FunctionPointer = 0;
	OriginalFunction = nullptr;
	hookedFunction = hookFunc;
	Stype = stype;

	if(bRunInit) Init();
}

template<typename Ret, typename ...Args>
Hook<Ret(Args...)>::Hook(unsigned long long addr, Ret(__fastcall* hookFunc)(Args...), bool bRunInit)
{
	pattern = "None";
	mask = "None";
	enabled = false;
	initialized = false;
	FunctionPointer = addr;
	OriginalFunction = nullptr;
	hookedFunction = hookFunc;
	Stype = FAST;

	if(bRunInit) Init();
}

template<typename Ret, typename ...Args>
inline Hook<Ret(Args...)>::Hook(Ret(__fastcall* ptr)(Args...), Ret(__fastcall* hookFunc)(Args...), bool bRunInit)
{
	pattern = "None";
	mask = "None";
	enabled = false;
	initialized = false;
	FunctionPointer = reinterpret_cast<unsigned long long>(ptr);
	OriginalFunction = nullptr;
	hookedFunction = hookFunc;
	Stype = FAST;

	if(bRunInit) Init();
}

template<typename Ret, typename ...Args>
Hook<Ret(Args...)>::~Hook()
{
	Disable();
	MH_RemoveHook((LPVOID)FunctionPointer);
	OriginalFunction = nullptr;
}

template<typename Ret, typename ...Args>
bool Hook<Ret(Args...)>::Init() {
	if (initialized) return false;
	if (FunctionPointer == 0) {
		unsigned long long tbase;
		unsigned __int64 tsize;
		if (!GetTextSection(tbase, tsize)) return false;
		switch (Stype)
		{
			case FAST:
				FunctionPointer = FindPattern(pattern, mask, tbase, tsize);
				break;
			case SAFE:
				FunctionPointer = FindPatternS(pattern, mask, tbase, tsize);
				break;
			case ALL:
				FunctionPointer = FindPatternAll(pattern, mask);
				break;
			default:
				FunctionPointer = FindPattern(pattern, mask, tbase, tsize);
				break;
		}
	}
	if (FunctionPointer == 0) return false;
	MH_STATUS ret = MH_CreateHook((LPVOID)FunctionPointer, hookedFunction, (void**)&OriginalFunction);
	initialized = ret == MH_OK;
	return ret == MH_OK;
}

template<typename Ret, typename ...Args>
inline void Hook<Ret(Args...)>::Create()
{
	if (initialized) return;
	assert(Init());
}

template<typename Ret, typename ...Args>
inline void Hook<Ret(Args...)>::Enable()
{
	if (!initialized) Create();
	if (!initialized || enabled) return;
	MH_QueueEnableHook((LPVOID)FunctionPointer);
	MH_ApplyQueued();
	enabled = true;
}

template<typename Ret, typename ...Args>
inline void Hook<Ret(Args...)>::Disable()
{
	if (!initialized) Create();
	if (!initialized || !enabled) return;
	MH_QueueDisableHook((LPVOID)FunctionPointer);
	MH_ApplyQueued();
	enabled = false;
}

template<typename Ret, typename ...Args>
inline Ret Hook<Ret(Args...)>::CallOriginalFunction(Args ...args)
{
	return OriginalFunction(std::forward<Args>(args)...);
}

template<typename Ret, typename ...Args>
inline unsigned long long Hook<Ret(Args...)>::FindPattern(const char* pattern, const char* mask, unsigned long long base, unsigned __int64 size)
{
	const unsigned __int64 patternLen = strlen(mask);
	if (patternLen == 0) {
		return 0;
	}

	// 1. Create the bad-character skip table
	std::vector<unsigned __int64> skipTable(256, patternLen);
	for (unsigned __int64 i = 0; i < patternLen - 1; ++i) {
		if (mask[i] != '?') {
			skipTable[static_cast<unsigned char>(pattern[i])] = patternLen - 1 - i;
		}
	}

	const unsigned long long searchEnd = base + size - patternLen;
	unsigned long long currentPos = base;

	while (currentPos <= searchEnd) {
		// 2. Compare from the end of the pattern backwards
		bool match = true;
		for (int j = patternLen - 1; j >= 0; --j) {
			if (mask[j] != '?' && pattern[j] != *(char*)(currentPos + j)) {
				// 3. On mismatch, use the skip table to jump forward
				// The character from the memory text determines the jump distance.
				const unsigned char mismatched_char = *(unsigned char*)(currentPos + patternLen - 1);
				currentPos += skipTable[mismatched_char];
				match = false;
				break;
			}
		}

		if (match) {
			return currentPos; // Found it
		}
	}

	return 0; // Not found
}

template<typename Ret, typename ...Args>
inline unsigned long long Hook<Ret(Args...)>::FindPatternS(const char* pattern, const char* mask, unsigned long long base, unsigned __int64 size)
{
	unsigned __int64 patternLen = strlen(mask);

	for (unsigned __int64 i = 0; i < size - patternLen; i++) {
		bool found = true;

		for (unsigned __int64 j = 0; j < patternLen; j++) {
			if (mask[j] != '?' && pattern[j] != *(char*)(base + i + j)) {
				found = false;
				break;
			}
		}

		if (found)
			return base + i;
	}

	return 0;
}

template<typename Ret, typename ...Args>
inline unsigned long long Hook<Ret(Args...)>::FindPatternAll(const char* signature, const char* mask)
{
	HMODULE hMods[1024];
	DWORD cbNeeded;
	HANDLE hProcess = GetCurrentProcess();
	if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded)) {
		for (unsigned int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++) {
			TCHAR szModName[MAX_PATH];

			if (GetModuleFileNameEx(hProcess, hMods[i], szModName, sizeof(szModName) / sizeof(TCHAR))) {
				std::wcout << L"Searching Module: " << szModName << std::endl;
			}

			uintptr_t base = reinterpret_cast<uintptr_t>(hMods[i]);

			MODULEINFO modInfo;
			GetModuleInformation(hProcess, hMods[i], &modInfo, sizeof(modInfo));
			unsigned __int64 size = modInfo.SizeOfImage;


			unsigned __int64 patternLen = strlen(mask);

			for (unsigned __int64 i = 0; i < size - patternLen; i++) {
				bool found = true;

				for (unsigned __int64 j = 0; j < patternLen; j++) {
					if (mask[j] != '?' && signature[j] != *(char*)(base + i + j)) {
						found = false;
						break;
					}
				}

				if (found)
					return base + i;
			}

		}

		return 0;
	}
}

template<typename Ret, typename ...Args>
inline unsigned long long Hook<Ret(Args...)>::GetModuleBase()
{
	return (unsigned long long)GetModuleHandle(NULL);
}

template<typename Ret, typename ...Args>
inline unsigned long long Hook<Ret(Args...)>::GetModuleSize()
{
	MODULEINFO info = {};
	GetModuleInformation(GetCurrentProcess(), GetModuleHandle(NULL), &info, sizeof(info));
	return (unsigned long long)info.SizeOfImage;
}

template<typename Ret, typename ...Args>
inline bool Hook<Ret(Args...)>::GetTextSection(unsigned long long& textBase, unsigned __int64& textSize)
{
	uintptr_t moduleBase = GetModuleBase();
	auto dos = (PIMAGE_DOS_HEADER)moduleBase;
	auto nt = (PIMAGE_NT_HEADERS)(moduleBase + dos->e_lfanew);

	auto section = IMAGE_FIRST_SECTION(nt);
	for (unsigned i = 0; i < nt->FileHeader.NumberOfSections; ++i, ++section)
	{
		if (strncmp((char*)section->Name, ".text", 5) == 0)
		{
			textBase = moduleBase + section->VirtualAddress;
			textSize = section->Misc.VirtualSize;
			return true;
		}
	}

	return false;
}

template<typename Ret, typename ...Args>
inline bool Hook<Ret(Args...)>::IsInitialized()
{
	return initialized;
}

template<typename Ret, typename ...Args>
inline bool Hook<Ret(Args...)>::IsEnabled()
{
	return enabled;
}

inline void DestroyHookInternal(void* hook)
{
	if (!hook) return;
	delete hook;
	hook = nullptr;
}
