#pragma once

// The following #defines disable a bunch of unused windows stuff. If you 
// get weird errors when trying to do some windows stuff, try removing some
// (or all) of these defines (it will increase build time though).
#ifndef FULL_WINDEFS
#define WIN32_LEAN_AND_MEAN
#define NORPC
#define NOPROXYSTUB
#define NOIMAGE
#define NOTAPE
#define NOGDICAPMASKS       // - CC_*, LC_*, PC_*, CP_*, TC_*, RC_
//#define NOVIRTUALKEYCODES // - VK_*
#define NOSYSMETRICS        // - SM_*
#define NOMENUS             // - MF_*
#define NOICONS             // - IDI_*
//#define NOKEYSTATES       // - MK_*
#define NOSYSCOMMANDS       // - SC_*
#define NORASTEROPS         // - Binary and Tertiary raster ops
#define OEMRESOURCE         // - OEM Resource values
#define NOATOM              // - Atom Manager routines
#define NOCLIPBOARD         // - Clipboard routines
#define NOCOLOR             // - Screen colors
#define NODRAWTEXT          // - DrawText() and DT_*
//#define NOGDI             // - All GDI defines and routines
//#define NOKERNEL          // - All KERNEL defines and routines
//#define NOUSER            // - All USER defines and routines
//#define NONLS             // - All NLS defines and routines
//#define NOMB              // - MB_* and MessageBox()
#define NOMEMMGR            // - GMEM_*, LMEM_*, GHND, LHND, associated routines
#define NOMETAFILE          // - typedef METAFILEPICT
#define NOMINMAX            // - Macros min(a,b) and max(a,b)
//#define NOMSG             // - typedef MSG and associated routines
#define NOOPENFILE          // - OpenFile(), OemToAnsi, AnsiToOem, and OF_*
#define NOSCROLL            // - SB_* and scrolling routines
#define NOSERVICE           // - All Service Controller routines, SERVICE_ equates, etc.
#define NOSOUND             // - Sound driver routines
#define NOTEXTMETRIC        // - typedef TEXTMETRIC and associated routines
//#define NOWH              // - SetWindowsHook and WH_*
//#define NOWINOFFSETS      // - GWL_*, GCL_*, associated routines
#define NOCOMM              // - COMM driver routines
#define NOKANJI             // - Kanji support stuff.
#define NOHELP              // - Help engine interface.
#define NOPROFILER          // - Profiler interface.
#define NODEFERWINDOWPOS    // - DeferWindowPos routines
#define NOMCX               // - Modem Configuration Extensions
//#define NOSHOWWINDOW      // - SW_*
//#define NOCTLMGR          // - Control and Dialog routines
//#define NOWINMESSAGES     // - WM_*, EM_*, LB_*, CB_*
//#define NOWINSTYLES       // - WS_*, CS_*, ES_*, LBS_*, SBS_*, CBS_*
#endif

#define STRICT

#include <Windows.h>