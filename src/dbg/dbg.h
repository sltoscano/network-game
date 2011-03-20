
#ifndef DEBUG_H
#define DEBUG_H

#ifdef _DEBUG
#define dbgprint(x, ...) wprintf(x, __VA_ARGS__)
#else
#define dbgprint(x, ...)
#endif

#ifdef _DEBUG

// autodetect CRT memory leaks
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#pragma warning(disable : 4005)
#include <crtdbg.h>
// can also use {,,msvcr100d.dll}_crtBreakAlloc in the debugger
#ifndef DEBUG_NEW
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif
#endif

#endif
