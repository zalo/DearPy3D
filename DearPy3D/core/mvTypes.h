#pragma once

typedef bool             b8;
typedef int              b32;

typedef float            f32;
typedef double           f64;

typedef signed char      i8;
typedef signed short     i16;
typedef signed int       i32;
#if defined(_MSC_VER) && !defined(__clang__)
typedef signed __int64   i64;
#else
typedef signed long long   i64;
#endif

typedef i8               s8;
typedef i16              s16;
typedef i32              s32;
typedef i64              s64;

typedef unsigned char    u8;
typedef unsigned short   u16;
typedef unsigned int     u32;
#if defined(_MSC_VER) && !defined(__clang__)
typedef unsigned __int64 u64;
#else
typedef unsigned long long   i64;
#endif

typedef s32 mvAssetID;

#define mv_internal static
#define mv_local_persist static
#define mv_global static
#define MV_INVALID_ASSET_ID -1

#define MV_VULKAN(x) if(x != VK_SUCCESS) throw std::runtime_error("runtime vulcan error")