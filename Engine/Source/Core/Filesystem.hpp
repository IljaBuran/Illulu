#pragma once

#include "Common.hpp"

#include "Vector.hpp"

#include <wrl/wrappers/corewrappers.h>

namespace Illulu::Filesystem
{
    using FileHandle = Microsoft::WRL::Wrappers::FileHandle;

    Vector<byte> ReadBinaryBlobFromFile(StringView path);
}