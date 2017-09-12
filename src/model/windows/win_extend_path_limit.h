#ifdef _WIN32

#ifndef WIN_EXTEND_PATH_LIMIT_H
#define WIN_EXTEND_PATH_LIMIT_H

#include <Windows.h>

#define MAKE_EXTENDED_PATH(path) ((path.getPathString().length() >= MAX_PATH) ? L"\\\\?\\" + path.getPathString() : path.getPathString())

#endif  // WIN_EXTEND_PATH_LIMIT_H

#else   // #ifdef _WIN32

#error "Macro _WIN32 isn't defined. Check target OS (required Windows) for this build"

#endif  // #ifdef _WIN32