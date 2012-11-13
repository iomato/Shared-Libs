#include "sciter-x.h"

#ifdef INSPECTOR_EXPORTS
#define INSPECTOR_API __declspec(dllexport)
#else
#define INSPECTOR_API __declspec(dllimport)
#endif

#pragma once

INSPECTOR_API VOID WINAPI SciterInspector(HELEMENT root);
INSPECTOR_API VOID WINAPI SciterWindowInspector(HWND hwndSciter);

