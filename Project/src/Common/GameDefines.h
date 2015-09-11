#pragma once


/*
* Frequently used includes, definitions and operations.
*/

#ifndef NOMINMAX
#define NOMINMAX
#endif

// windows and VS specific stuff
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#ifdef _MSC_VER
#pragma warning (disable:4786)
#pragma warning (disable:4099)
#pragma warning (disable:4800)
#pragma warning (disable:4308)	// boost
#pragma warning (disable:4244)	// boost
#endif


#define GX_DEBUG_INFO
//#define ENABLE_BLOOD_EFFECT