#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <Windows.h>
#include <comdef.h>

#include "String.hpp"

inline String TranslateHResult(HRESULT hRes)
{
	_com_error err(hRes);
	return err.ErrorMessage();
}