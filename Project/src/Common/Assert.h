#pragma once

#ifdef _DEBUG
#define ASSERTS_ENABLED
#endif

#ifdef ASSERTS_ENABLED

void gameAssert(const char*, const char*, int, const char*, ...);

#define GX_ASSERT(e, ...) ((void) ((e) ? 0 : gameAssert (#e, __FILE__, __LINE__ , __FUNCTION__, __VA_ARGS__)))
#define GX_DEBUG_BREAK_IF(cond) if (cond) { __debugbreak(); }
///#define GX_DEBUG_BREAK_IF(cond) if (cond) { __asm int 3 }

#else

#define GX_ASSERT(e, ...)
#define GX_DEBUG_BREAK_IF(cond)

#endif
