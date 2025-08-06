/*----------------------------------------------------------------------------*/
/*                                                                            */
/*    Copyright (c) Aaron Wilk 2025, All rights reserved.                     */
/*                                                                            */
/*    Module:     UtilitiesImplementation.hpp				      */
/*    Author:     Aaron Wilk                                                  */
/*    Created:    14 July 2025                                                */
/*                                                                            */
/*    Revisions:  V0.1                                                        */
/*                                                                            */
/*----------------------------------------------------------------------------*/

#pragma once
#include "GameFunctions.hpp"
#include "Offsets.hpp"
#include <windows.h>
#include <tlhelp32.h>
#include "UnrealContainers.hpp"
#include "../SDK/Basic.hpp"
#include "../SDK/Engine_classes.hpp"
#include "../SDK/Engine_structs.hpp"
#include "../SDK/BrickRigs_classes.hpp"
#include "../SDK/CoreUObject_classes.hpp"
#include "../SDK/UMG_classes.hpp"

namespace __utilities
{
	//Define mock structures to work with UE Internals
	namespace __data
	{
		 struct FSHFiller
		 {
		 	uint8_t padding[0x10];
		 };

		 struct FStreamableHandle : FSHFiller
		 {
		 	bool bLoadCompleted;
		 	bool bReleased;
		 	bool bCanceled;
		 	bool bStalled;
		 	bool bReleaseWhenLoaded;
		 	bool bIsCombinedHandle;
		 	uint8_t pad[50];
		 	UC::FString DebugName;
		 	int priority;
		 	int StreamablesLoading;
		 	int CompletedChildCount;
		 	int CanceledChildCount;
		 	uint8_t pad1[48];
		 	void* OwningManager;
		 };

		 static_assert(sizeof(FStreamableHandle) == 0xA0);

		 struct SharedPtr
		 {
		 	FStreamableHandle* ptr;
		 	uint8_t pad[0x8];
		 };
	}

	inline bool IsInGameThread()
	{
		return GetCurrentThreadId() == *reinterpret_cast<unsigned int*>(G_GGAMETHREADID);
	}

 	//Gets the FPlatformFileManager
	inline void* GetPlatformFile()
	{
		return CallGameFunction<void*, void*>(F_GETPLATFORMFILE, CallGameFunction<void*>(F_GETPLATFORMFILEMANAGER));
	}

	//Sets the path of a FSoftObjectPath
	inline void SetPath(SDK::FakeSoftObjectPtr::FSoftObjectPath* This, SDK::FName pathname)
	{
		return CallGameFunction<void, SDK::FakeSoftObjectPtr::FSoftObjectPath*, SDK::FName>(F_SETPATH, This, pathname);
	}

	//Gets every asset file in the virtual file system
	inline SDK::TArray<SDK::FString> GetVFSFiles()
	{
		SDK::FString gamepath = SDK::UBlueprintPathsLibrary::ProjectContentDir();
		SDK::TArray<SDK::FString> ret;
		CallGameFunction<void, void*, SDK::TArray<SDK::FString>*, const wchar_t*, const wchar_t*>(F_FINDFILESRECURSIVELY, GetPlatformFile(), &ret, gamepath.CStr(), L".uasset");
		return ret;
	}

	//Basic std::string to std::wstring
	inline std::wstring _to_wstring(const std::string& str)
	{
		int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
		std::wstring wstr(size_needed, 0);
		MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstr[0], size_needed);
		wstr.pop_back(); // remove null terminator
		return wstr;
	}

	//Finds the asset path of the class in string form
	inline std::wstring FindClassAssetPath(const char* classname)
	{
		std::wstring universal = _to_wstring(classname);
		universal = universal.substr(universal.find_first_of('B'));//remove the ABP, UBP, UWBP.
		//std::wcout << "Attempting Load: " << universal << std::endl;
		std::wstring lookfor = universal;
		lookfor.pop_back();
		lookfor.pop_back();
		lookfor.append(L".");//Remove the _C and add a dot so that we know when the name ends.
		std::wstring res = std::wstring(L"NONE");
		SDK::TArray<SDK::FString> files = GetVFSFiles();
		for (int i = 0; i < files.Num(); i++)
		{
			if (files[i].ToWString().find(std::wstring(lookfor)) != std::string::npos)
			{
				//../../../BrickRigs/Content/BrickRigs/UI/Properties/WBP_PropertiesPanel.uasset
				std::wstring original = files[i].ToWString();
				const std::wstring target = L"BrickRigs/";

				// Find the first occurrence of "BrickRigs/"
				size_t firstBrickRigs = original.find(target);
				if (firstBrickRigs == std::wstring::npos) {
					break;
				}

				// Find the next "BrickRigs/" after the first one (skipping "BrickRigs/Content/")
				size_t secondBrickRigs = original.find(target, firstBrickRigs + target.length());
				if (secondBrickRigs == std::wstring::npos) {
					break;
				}
				// Extract from the second "BrickRigs/"
				std::wstring result = original.substr(secondBrickRigs);

				// Remove the ".uasset" extension
				size_t dotPos = result.rfind('.');
				if (dotPos != std::wstring::npos) {
					result = result.substr(0, dotPos);
				}

				//Find the package name. Everything after the last slash
				size_t slashpos = result.find_last_of('/');
				std::wstring packname = L"";
				if (slashpos != std::wstring::npos)
				{
					packname = result.substr(slashpos + 1);
				}

				result = L"/Game/" + result;

				result.append(L".");
				result.append(packname);
				result.append(L"_C");

				//std::wcout << result << std::endl;

				res = result;
				break;
			}
		}

		if (res == L"NONE") { std::cout << "Failed to Find Class!" << std::endl; return L"NONE"; }

		return res.c_str();
	}

	inline void* GetStreamableManager()
	{
		ASSERT(SDK::UBrickAssetManager::Get()->StreamableManager != nullptr, "Streamable manager was NULL! This is a Brick Rigs Error.");
		return (void*)SDK::UBrickAssetManager::Get()->StreamableManager;
	}

	//Calls LoadSynchronous on the SoftClass pointer. do not directly, call use the GetUClass() macro
	inline void RequestSyncLoad(SDK::TSoftClassPtr<SDK::UClass>* sftptr)
	{
		CallGameFunction<SDK::UObject*, SDK::FSoftObjectPtr*>(F_LOADSYNCHRONOUS, sftptr);
	}

	//Attempts to load a Class async. Will fail if on the MainThread. do not call directly, use the GetUClass() macro.
	inline void RequestAsyncLoad(SDK::FakeSoftObjectPtr::FSoftObjectPath* path)
	{
		ASSERT(!IsInGameThread(), "RequestAsyncLoad cannot be called on the Main Thread! Use ");
		SDK::UGunBrick* BrickHandeler = static_cast<SDK::UGunBrick*>(SDK::UGameplayStatics::SpawnObject(SDK::UGunBrick::StaticClass(), SDK::UWorld::GetWorld()));
		SDK::TDelegate<void(void)> dele = SDK::TDelegate<void(void)>();
		uintptr_t fnAddress = F_MARKBRICKBURNT;//Set delegate address to the mark brick burnt function.
		// Manually build the 16-byte function pointer representation
		uint64_t funcBlob[2] = { fnAddress, 0 };
		SDK::TDelegate<void(void)>* delenew = CallGameFunction<SDK::TDelegate<void(void)>*, SDK::TDelegate<void(void)>*, SDK::UGunBrick*, uint64_t*>(F_CREATEUOBJECT, &dele, BrickHandeler, funcBlob);
		__data::SharedPtr ptr{};
		ptr.ptr = nullptr;
		UC::FString str = UC::FString(L"LoadAssetList");
		__data::SharedPtr* ptrret = CallGameFunction<__data::SharedPtr*, void*, __data::SharedPtr*, const SDK::FakeSoftObjectPtr::FSoftObjectPath*, SDK::TDelegate<void __cdecl(void)>*, int, bool, bool, SDK::FString*>(F_REQUESTASYNCLOAD, GetStreamableManager(), &ptr, path, delenew, 0, false, false, &str);
		int max = 0;
		while (max <= 10)
		{
			Sleep(50);
			if (BrickHandeler->IsBrickBurnt()) break;
			max++;
		}
		CallGameFunction<__int64, void*, float, bool>(F_WAITUNTILCOMPLETE, ptrret->ptr, 0.0, 0);//Safe to call and finalize the load.
	}

	/// <summary>
	/// Loads a blueprint class from the SDK
	/// </summary>
	/// <param name="classname">String representation of the class the SDK:: will be filtered out if present, IE: SDK::ABP_CarElevator_C, or ABP_CarElevator_C.</param>
	inline void AttemptLoadClass(const char* classname)
	{
		SDK::TSoftClassPtr<SDK::UClass> ptr = SDK::TSoftClassPtr<SDK::UClass>();
		//For some reason as to which i have no idea, the FString HAS to be created inside the function parmeter that converts it to a SDK::FName
		const SDK::FName path = SDK::UKismetStringLibrary::Conv_StringToName(SDK::FString(FindClassAssetPath(classname).c_str()));
		SetPath(&ptr.ObjectID, path);
		if (IsInGameThread())
		{
			//Load the class using the blocking thread.
			RequestSyncLoad(&ptr);
			return;
		}
		RequestAsyncLoad(&ptr.ObjectID);
		return;
	}
}

//Gets the UClass from the string and templates representation of the class. do not use. Use the GetUClass() macro.
template<typename T>
inline SDK::UClass* GetClassInternal(const char* clsobjname)
{
	SDK::UClass* objcls = T::StaticClass();
	if (!objcls) __utilities::AttemptLoadClass(clsobjname);
	objcls = T::StaticClass();
	ASSERT(objcls != nullptr, std::string("Class: ") + std::string(clsobjname) + std::string(" Could not be loaded!"));
	return objcls;
}

//Class safe version of SDK::UGameplayStatics::SpawnObject. Use the SpawnObject() macro instead of this function.
template<typename T>
inline T* SpawnObjectInternal(SDK::UObject* outerobj, const char* objclsname)
{
	SDK::UClass* objcls = GetClassInternal<T>(objclsname);
	return static_cast<T*>(SDK::UGameplayStatics::SpawnObject(objcls, outerobj));
}

template<typename T>
inline T* CreateWidgetInternal(SDK::TSubclassOf<SDK::UUserWidget> UserWidgetClass, const char* WidgetClassName)
{

	if (UserWidgetClass == nullptr) UserWidgetClass = GetClassInternal<T>(WidgetClassName);
	return static_cast<T*>(CallGameFunction<SDK::UUserWidget*, SDK::UWorld*, SDK::TSubclassOf<SDK::UUserWidget>, SDK::FName>(F_CREATEWIDGET, SDK::UWorld::GetWorld(), UserWidgetClass, SDK::FName()));
}

template<typename T>
inline T* SpawnActorInternal(SDK::AActor* outeract, const char* objclsname)
{
	ASSERT(outeract == nullptr, "Actors only spawned in refrence to another actor must not be null!");
	SDK::FTransform transform = outeract->GetTransform();
	SDK::UClass* objcls = GetClassInternal<T>(objclsname);
	SDK::ESpawnActorCollisionHandlingMethod method = SDK::ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SDK::AActor* act = SDK::UGameplayStatics::BeginDeferredActorSpawnFromClass(SDK::UWorld::GetWorld(), objcls, transform, method, outeract);
	return static_cast<T*>(SDK::UGameplayStatics::FinishSpawningActor(act, transform));
}

template<typename T>
inline T* SpawnActorInternal(SDK::FTransform transform, SDK::AActor* outeract, const char* objclsname, bool deferred)
{
	SDK::UClass* objcls = GetClassInternal<T>(objclsname);
	SDK::ESpawnActorCollisionHandlingMethod method = SDK::ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SDK::AActor* act = SDK::UGameplayStatics::BeginDeferredActorSpawnFromClass(SDK::UWorld::GetWorld(), objcls, transform, method, outeract);
	SDK::UGameplayStatics::FinishSpawningActor(act, transform);
	return static_cast<T*>(act);
}

template<typename T>
inline T* SpawnActorInternal(SDK::FVector position, SDK::AActor* outeract, const char* objclsname)
{
	SDK::FRotator rotator = SDK::FRotator();
	if (!outeract)
	{
		rotator = SDK::FRotator(0, 0, 0);
	}
	else rotator = outeract->K2_GetActorRotation();
	SDK::FTransform transform = SDK::FTransform();
	transform.Translation = position;
	transform.Scale3D = SDK::FVector(1, 1, 1);
	SDK::FQuat formed = SDK::FQuat();
	SDK::FQuat* ptr = CallGameFunction<SDK::FQuat*, SDK::FRotator*, SDK::FQuat*>(F_QUATERNION, &rotator, &formed);
	formed = *ptr;
	transform.Rotation = formed;
	SDK::UClass* objcls = GetClassInternal<T>(objclsname);
	SDK::ESpawnActorCollisionHandlingMethod method = SDK::ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SDK::AActor* act = SDK::UGameplayStatics::BeginDeferredActorSpawnFromClass(SDK::UWorld::GetWorld(), objcls, transform, method, outeract);
	SDK::UGameplayStatics::FinishSpawningActor(act, transform);
	return static_cast<T*>(act);
}

template<typename T>
inline T* SpawnActorInternal(SDK::FVector position, SDK::FRotator rotation, SDK::AActor* outeract, const char* objclsname)
{
	SDK::FTransform transform = SDK::FTransform();
	transform.Translation = position;
	transform.Scale3D = SDK::FVector(1, 1, 1);
	SDK::FQuat formed = SDK::FQuat();
	formed = &CallGameFunction<SDK::FQuat*, SDK::FRotator*, SDK::FQuat*>(F_QUATERNION, &rotation, &formed);
	transform.Rotation = formed;
	SDK::UClass* objcls = GetClassInternal<T>(objclsname);
	SDK::ESpawnActorCollisionHandlingMethod method = SDK::ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SDK::AActor* act = SDK::UGameplayStatics::BeginDeferredActorSpawnFromClass(SDK::UWorld::GetWorld(), objcls, transform, method, outeract);
	SDK::UGameplayStatics::FinishSpawningActor(act, transform);
	return static_cast<T*>(act);
}
