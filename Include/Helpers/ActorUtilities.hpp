/*----------------------------------------------------------------------------*/
/*                                                                            */
/*    Copyright (c) Aaron Wilk 2025, All rights reserved.                     */
/*                                                                            */
/*    Module:     ActorUtilities.hpp						                  */
/*    Author:     Aaron Wilk                                                  */
/*    Created:    14 July 2025                                                */
/*                                                                            */
/*    Revisions:  V0.1                                                        */
/*                                                                            */
/*----------------------------------------------------------------------------*/

#pragma once

#include "UtilitiesImplementation.hpp"


/// <summary>
/// Destroys an AActor using internal UE systems.
/// </summary>
/// <param name="actor">Pointer to the actor to be deleted</param>
/// <returns>None</returns>
#define DestroyActor(actor) actor->K2_DestroyActor()

/// <summary>
/// Spawns a new AActor using internal UE systems. Use when creating AActors
/// The actor will be created with the specified parent and spawn with its location and rotation
/// If a parent is not specified, its location and rotation will be 0,0,0
/// </summary>
/// <param name="cls">Class of the new actor. Not the UClass. Ex: SDK::ABrickCharacte </param>
/// <param name="parent">Pointer to the parent of the new actor.</param>
/// <returns>A pointer to the new actor</returns>
#define SpawnActor(cls, parent) SpawnActorInternal<cls>(parent, #cls)

/// <summary>
/// Spawns a new AActor using internal UE systems. Use when creating AActors
/// The actor will be created with the specified parent and spawn with the specified actors location and rotation
/// </summary>
/// <param name="cls">Class of the new actor. Not the UClass. Ex: SDK::ABrickCharacter </param>
/// <param name="parent">Pointer to the parent of the new actor./param>
/// <param name="actor">Pointer to the actor to get the location for the new actor</param>
/// <returns>A pointer to the new actor</returns>
#define SpawnActorAt(cls, parent, actor) SpawnActorInternal<cls>(actor->K2_GetActorLocation(), actor->K2_GetActorRotation(), parent, #cls)

/// <summary>
/// Spawns a new AActor using internal UE systems. Use when creating AActors
/// The actor will be created with the specified parent and spawn at the specified location
/// </summary>
/// <param name="cls">Class of the new actor. Not the UClass. Ex: SDK::ABrickCharacter </param>
/// <param name="parent">Pointer to the parent of the new actor./param>
/// <param name="vector">Location of the new actor</param>
/// <returns>A pointer to the new actor</returns>
#define SpawnActorVec(cls, parent, vector) SpawnActorInternal<cls>(vector, parent, #cls)

/// <summary>
/// Spawns a new AActor using internal UE systems. Use when creating AActors
/// The actor will be created with the specified parent and spawn with the specified location and rotation
/// </summary>
/// <param name="cls">Class of the new actor. Not the UClass. Ex: SDK::ABrickCharacter </param>
/// <param name="parent">Pointer to the parent of the new actor./param>
/// <param name="vector">Location of the new actor</param>
/// <param name="rotator">Rotation of the new actor</param>
/// <returns>A pointer to the new actor</returns>
#define SpawnActorLoc(cls, parent, vector, rotator) SpawnActorInternal<cls>(vector, rotator, parent, #cls)

/// <summary>
/// Spawns a new AActor using internal UE systems. Use when creating AActors
/// The actor will be created with the specified parent and spawn with the specified location and rotation
/// </summary>
/// <param name="cls">Class of the new actor. Not the UClass. Ex: SDK::ABrickCharacter </param>
/// <param name="parent">Pointer to the parent of the new actor./param>
/// <param name="transform">Transform of the new actor</param>
/// <returns>A pointer to the new actor</returns>
#define SpawnActorTrans(cls, parent, transform) SpawnActorInternal<cls>(transform, parent, #cls, false)

/// <summary>
/// Spawns a new AActor using internal UE systems and be deferred until FinishSpawningActor has been called. Use when creating AActors
/// The actor will be created with the specified parent and spawn with the specified location and rotation
/// </summary>
/// <param name="cls">Class of the new actor. Not the UClass. Ex: SDK::ABrickCharacter </param>
/// <param name="parent">Pointer to the parent of the new actor./param>
/// <param name="transform">Transform of the new actor</param>
/// <returns>A pointer to the new actor</returns>
#define DeferredSpawnActorTrans(cls, parent, transform) SpawnActorInternal<cls>(transform, parent, #cls, true)

/// <summary>
/// Finishes spawning a deferred actor created with DeferredSpawnActorTrans
/// The actor will be created with the specified transform
/// </summary>
/// <param name="actor">Pointer to the actor to finish spawning</param>
/// <param name="transform">Transform of the new actor</param>
/// <returns>A pointer to the new actor</returns>
#define FinishSpawningDeferredActor(actor, transform) SDK::UGameplayStatics::FinishSpawningActor(actor, transform);