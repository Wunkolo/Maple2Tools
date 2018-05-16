#pragma once

#include <cstddef>
#include <cstdint>

#include "../Util/Structure.hpp"

namespace Maple2
{
#pragma pack(1)

struct PackFileHeaderVer1
{
private:
	std::uint32_t Unknown00; // Pad
public:

	std::uint64_t FATCompressedSize;      // TableCompressedSize (Key Index)
	std::uint64_t FATEncodedSize;         // TableEncodedSize

	std::uint64_t FileListSize;           // FileListSize
	std::uint64_t FileListCompressedSize; // FileListCompressedSize (Key Index)
	std::uint64_t FileListEncodedSize;    // FileListEncodedSize

	std::uint64_t TotalFiles;             // Total Files
	std::uint64_t FATSize;                // Uncompressed size
};

struct FATEntry
{
private:
	std::uint32_t Pad;
public:
	std::uint32_t FileIndex;
	std::uint64_t Unknown; // 0xEE000009
	std::uint64_t Offset;
	std::uint64_t EncodedSize;
	std::uint64_t CompressedSize;
	std::uint64_t Size;
};

#pragma pack()
}