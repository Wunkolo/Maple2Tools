#pragma once
#include <cstddef>
#include <string>
#include <map>
#include <experimental/filesystem>

namespace Maple2
{
namespace Util
{
std::map<std::size_t, std::experimental::filesystem::path> ParseFileList(
	const std::string& FileList
);
}
}
