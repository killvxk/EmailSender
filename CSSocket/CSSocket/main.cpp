#include"httpserver.h"
#include "precomp.h"
int __cdecl wmain(
	int argc,
	wchar_t * argv[]
) {
	const int cnt = 1;
	wchar_t * str[cnt] = { L"http://localhost:8080/socket/" };
	startHttpservives(cnt , str);
	return 0;
}

