#pragma once
#include <cstdint>
#include <SDK/CoreUObject_structs.hpp>

struct FSharedRef
{
    void* obj;
    uint8_t refenceController[0x8];
};
static_assert(sizeof(FSharedRef) == 0x10);

struct FNumericBrickPropertyValue
{
    SDK::FVector Data;
    unsigned __int8 NumUsed;
};
static_assert(sizeof(FNumericBrickPropertyValue) == 0x10);

struct FNumericBrickPropertyRange
{
    FNumericBrickPropertyValue Min;
    FNumericBrickPropertyValue Max;
};
static_assert(sizeof(FNumericBrickPropertyRange) == 0x20);


struct FOptional
{
    FNumericBrickPropertyRange Value;
    bool bIsSet;
    uint8_t pad[3];
};
static_assert(sizeof(FOptional) == 0x24);

struct FBrickPropAttribute //const TBrickPropAttribute<FNumericBrickPropertyRange> ValueRange;
{
    FOptional opt;
    uint8_t padn[4];
    uint8_t pad[0x10];
};
static_assert(sizeof(FBrickPropAttribute) == 0x38);

struct FNumericBrickPropertyBase
{
    uint8_t pad[0x18];
    uint8_t pad1[0x18];
    FBrickPropAttribute Range;
    uint8_t pad2[0x18];
};
static_assert(sizeof(FNumericBrickPropertyBase) == 0x80);