#pragma once

#include <cstddef>
#include <cstdint>

#include "../Util/Structure.hpp"

namespace Maple2
{
#pragma pack( push, 1 )

struct PackFileHeaderVer1 // MS2F
{
public:
	std::uint32_t Pad;                    // Always zero

	std::uint64_t FATCompressedSize;      // DEFLATE length
	std::uint64_t FATEncodedSize;         // Base64 length

	std::uint64_t FileListSize;           // Uncompressed size
	std::uint64_t FileListCompressedSize; // DEFLATE length
	std::uint64_t FileListEncodedSize;    // Base64 length

	std::uint64_t TotalFiles;             // Total Files
	std::uint64_t FATSize;                // Uncompressed size
};
static_assert(
	sizeof(PackFileHeaderVer1) == 0x3C,
	"sizeof(PackFileHeaderVer1) != 0x3C"
);

struct PackFileHeaderVer2 // NS2F
{
public:
	std::uint32_t TotalFiles;

	std::uint64_t FATCompressedSize;      // DEFLATE length
	std::uint64_t FATEncodedSize;         // Base64 length
	std::uint64_t FATSize;                // Uncompressed size

	std::uint64_t FileListB64Size;
	std::uint64_t FileListEncodedSize;    // Base64 length
	std::uint64_t FileListSize;           // Uncompressed size
};
static_assert(
	sizeof(PackFileHeaderVer2) == 0x34,
	"sizeof(PackFileHeaderVer2) != 0x34"
);

struct FATEntry
{
public:
	std::uint32_t Pad;
	std::uint32_t FileIndex;
	std::uint64_t Unknown;                 // 0xEE000009
	std::uint64_t Offset;
	std::uint64_t EncodedSize;
	std::uint64_t CompressedSize;
	std::uint64_t Size;
};
static_assert(
	sizeof(FATEntry) == 0x30,
	"sizeof(FATEntry) != 0x30"
);


#pragma pack( pop )
}
