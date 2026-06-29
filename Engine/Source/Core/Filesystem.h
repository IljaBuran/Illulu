#pragma once

#include "Common.h"

#include "Vector.h"

#include <wrl/wrappers/corewrappers.h>

namespace Illulu::Filesystem
{
	using FileHandle = Microsoft::WRL::Wrappers::FileHandle;

	Vector<byte> ReadBinaryBlobFromFile(String path);
}