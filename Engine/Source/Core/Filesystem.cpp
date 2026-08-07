#include "Filesystem.hpp"

namespace Illulu::Filesystem
{
	Vector<byte> ReadBinaryBlobFromFile(StringView path)
	{
		CREATEFILE2_EXTENDED_PARAMETERS extendedParams
		{
			.dwSize{sizeof(CREATEFILE2_EXTENDED_PARAMETERS)},
			.dwFileAttributes{FILE_ATTRIBUTE_NORMAL},
			.dwFileFlags{FILE_FLAG_SEQUENTIAL_SCAN},
			.dwSecurityQosFlags{SECURITY_ANONYMOUS},
			.lpSecurityAttributes{nullptr},
			.hTemplateFile{nullptr}
		};
	
		FileHandle fileHandle(CreateFile2(path.data(), GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING, &extendedParams));
		ILL_ASSERT(fileHandle.IsValid());

		FILE_STANDARD_INFO fileInfo{};

		ILL_VERIFY(GetFileInformationByHandleEx(fileHandle.Get(), FileStandardInfo, &fileInfo, sizeof(fileInfo)));

		// we won't support files bigger than 4GiBs;
		ILL_ASSERT(fileInfo.EndOfFile.HighPart == 0);

		u32 fileSize = fileInfo.EndOfFile.LowPart;
		Vector<byte> data(fileSize);
	
		UNUSED DWORD bytesRead{};
		ILL_VERIFY(ReadFile(fileHandle.Get(), data.data(), fileSize, &bytesRead, nullptr));

		ILL_ASSERT(bytesRead != 0);

		return data;
	}
}
