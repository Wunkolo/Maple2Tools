#include <Maple2/Util.hpp>

#include <regex>

#include <cryptopp/aes.h>
#include <cryptopp/base64.h>
#include <cryptopp/modes.h>
#include <cryptopp/filters.h>
#include <cryptopp/zlib.h>

namespace fs = std::experimental::filesystem;

namespace Maple2
{
namespace Util
{
std::string DecryptStream(
	const std::string& Encoded,
	const std::uint8_t IV[16],
	const std::uint8_t Key[32],
	bool Compressed
)
{
	std::string Decrypted;

	CryptoPP::CTR_Mode<CryptoPP::AES>::Decryption Decryptor;
	Decryptor.SetKeyWithIV(Key, 32, IV);

	if( Compressed )
	{
		CryptoPP::StringSource(
			Encoded,
			true,
			new CryptoPP::Base64Decoder(
				new CryptoPP::StreamTransformationFilter(
					Decryptor,
					new CryptoPP::ZlibDecompressor(
						new CryptoPP::StringSink(Decrypted)
					)
				)
			)
		);
	}
	else
	{
		CryptoPP::StringSource(
			Encoded,
			true,
			new CryptoPP::Base64Decoder(
				new CryptoPP::StreamTransformationFilter(
					Decryptor,
					new CryptoPP::StringSink(Decrypted)
				)
			)
		);
	}

	return Decrypted;
}

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
