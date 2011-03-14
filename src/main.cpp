
#include "common.h"
#include "glmain.h"

#ifndef _DEBUG
#include <comdef.h>
#endif

#ifdef _DEBUG
int main(int argc, char* argv[])
{
#else
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	REF(hInstance);
	REF(hPrevInstance);
	REF(pCmdLine);
	REF(nCmdShow);
	int argc = 1;
	char* argv[1];
	argv[0] = "main";
#endif
	return glmain(argc, argv);
}
