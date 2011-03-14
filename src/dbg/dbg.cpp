
#include "dbg.h"


#ifdef _DEBUG
// init CRT memory leak detection before main is called
struct init {
	init() { _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF ); }
} _init;
#endif
