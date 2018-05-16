#pragma once

#include <cstddef>
#include <cstdint>

#include "../Util/Structure.hpp"

namespace Maple2
{
#pragma pack(1)

struct PackFileHeaderVer1 // MS2F
{
private:
	std::uint32_t Unknown00;              // Always zero
public:

	std::uint64_t FATCompressedSize;      // DEFLATE length
	std::uint64_t FATEncodedSize;         // Base64 length

	std::uint64_t FileListSize;           // Uncompressed size
	std::uint64_t FileListCompressedSize; // DEFLATE length
	std::uint64_t FileListEncodedSize;    // Base64 length

	std::uint64_t TotalFiles;             // Total Files
	std::uint64_t FATSize;                // Uncompressed size
};

struct PackFileHeaderVer2 // NS2F
{
	std::uint32_t TotalFiles;
	std::uint64_t FATCompressedSize;      // DEFLATE length
	std::uint64_t FATEncodedSize;         // Base64 length
	std::uint64_t FATSize;                // Uncompressed size

	std::uint64_t FileListB64Size;
	std::uint64_t FileListEncodedSize;    // Base64 length
	std::uint64_t FileListSize;           // Uncompressed size
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
