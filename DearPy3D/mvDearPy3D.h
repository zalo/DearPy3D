#pragma once

#include "mvConfig.h"

// Warnings
#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable: 26495)    // [Static Analyzer] Variable 'XXX' is uninitialized. Always initialize a member variable (type.6).
#endif
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
#if __has_warning("-Wzero-as-null-pointer-constant")
#pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"
#endif
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas"          // warning: unknown option after '#pragma GCC diagnostic' kind
#pragma GCC diagnostic ignored "-Wclass-memaccess"  // [__GNUC__ >= 8] warning: 'memset/memcpy' clearing/writing an object of type 'xxxx' with no trivial copy-assignment; use assignment or value-initialization instead
#endif

//-----------------------------------------------------------------------------
// [SECTION] forward declarations & basic types
//-----------------------------------------------------------------------------

// forward declarations
struct mvGraphics;
struct mvBuffer;
struct mvBufferSpecification;
struct mvDescriptorSpec;
struct mvDescriptorSetLayout;
struct mvDescriptor;
struct mvDescriptorSet;
struct mvDescriptorManager;
struct mvMaterialData;
struct mvMaterial;
struct mvMaterialManager;
struct mvRendererContext;
struct mvMesh;
struct mvShader;
struct mvVertexLayout;
struct mvPipelineSpec;
struct mvPipeline;
struct mvPipelineManager;
struct mvPassSpecification;
struct mvPass;
struct mvTexture;
struct mvTextureManager;

// enums/flags
typedef int mvVertexElementType;

// types
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

// Helper Macros
#ifndef MV_ASSERT
#include <assert.h>
#define MV_ASSERT(_EXPR) assert(_EXPR) // You can override the default assert handler by editing mvConfig.h
#define MV_VULKAN(x) assert(x == VK_SUCCESS)
#endif

#if defined(_WIN32)
#define VK_USE_PLATFORM_WIN32_KHR
#endif