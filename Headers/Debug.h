#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <cstdarg>
#include <wchar.h>
#include <Windows.h>

inline void DebugWindow(const WCHAR* format, ...) {
	va_list args;
	va_start(args, format);

	WCHAR msg[1024];
	wvsprintf(msg, format, args);
	MessageBox(0, msg, L"", MB_OK);

	va_end(args);
}

#endif  // _DEBUG_H_
