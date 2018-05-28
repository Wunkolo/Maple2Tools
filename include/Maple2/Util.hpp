#pragma once
#include <cstddef>
#include <string>
#include <map>
#include <experimental/filesystem>

namespace Maple2
{
namespace Util
{

std::string DecryptStream(
	const std::string& Encoded,
	const std::uint8_t IV[16],
	const std::uint8_t Key[32],
	bool Compressed = false
);

std::map<std::size_t, std::experimental::filesystem::path> ParseFileList(
	const std::string& FileList
);
}
}
