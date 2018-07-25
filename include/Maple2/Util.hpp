#pragma once
#include <cstddef>
#include <string>
#include <fstream>
#include <map>
#include <tuple>
#include <experimental/filesystem>

namespace Maple2
{
namespace Util
{

std::tuple<
	std::string, // Encrypted data
	std::uint64_t, // Compressed Size
	std::uint64_t  // Encoded Size
> EncryptString(
	const std::string& Data,
	const std::uint8_t IV_LUT[128][16],
	const std::uint8_t Key_LUT[128][32],
	bool Compress = true
);

std::tuple<
	std::string, // Encrypted data
	std::uint64_t, // Compressed Size
	std::uint64_t  // Encoded Size
> EncryptFile(
	std::ifstream& FileStream,
	const std::uint8_t IV_LUT[128][16],
	const std::uint8_t Key_LUT[128][32],
	bool Compress = true
);

void DecryptStream(
	const void* Encoded,
	std::size_t EncodedSize,
	const std::uint8_t IV[16],
	const std::uint8_t Key[32],
	void* Decoded,
	std::size_t DecodedSize,
	bool Compressed = false
);

std::map<std::size_t, std::experimental::filesystem::path> ParseFileList(
	const std::string& FileList
);
}
}
