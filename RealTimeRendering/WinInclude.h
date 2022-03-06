#pragma once

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include <Windows.h>
#include <shlwapi.h>
#include <Shlobj.h>
#include <pathcch.h>
#include <dxgi1_6.h>
#include <d3d12.h>
#include <DirectXMath.h>
