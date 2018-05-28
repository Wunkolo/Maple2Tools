#include <Maple2/Util.hpp>

#include <regex>
namespace fs = std::experimental::filesystem;

namespace Maple2
{
namespace Util
{
std::map<std::size_t, fs::path> ParseFileList(const std::string& FileList)
{
	std::map<std::size_t, fs::path> FileListEntries;
	// Split based on \r\n
	static const std::regex RegExNewline("[\r\n]+");

	std::sregex_token_iterator TokenIter(
		FileList.begin(),
		FileList.end(),
		RegExNewline,
		-1
	);

	const std::sregex_token_iterator TokenEnd;

	for( ; TokenIter != TokenEnd; ++TokenIter )
	{
		const std::string CurFileLine = (*TokenIter).str();
		const fs::path HeaderFiledex = CurFileLine.substr(
			0,
			CurFileLine.find_first_of(',')
		);
		const fs::path FileName = CurFileLine.substr(
			CurFileLine.find_last_of(',') + 1
		);

		FileListEntries[std::stoull(HeaderFiledex)] = FileName;
	}
	return FileListEntries;
}
}
}
