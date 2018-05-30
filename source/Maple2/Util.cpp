#include <Maple2/Util.hpp>

#include <regex>

#include <cryptopp/files.h>
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
std::tuple<
	std::string, // Encrypted data
	std::uint64_t, // Compressed Size
	std::uint64_t  // Encoded Size
> EncryptString(
	const std::string& Data,
	const std::uint8_t IV_LUT[128][16],
	const std::uint8_t Key_LUT[128][32],
	bool Compress
)
{
	std::string Encoded;
	std::uint64_t CompressedSize;

	CryptoPP::CTR_Mode<CryptoPP::AES>::Encryption Encryptor;

	// Encrypt: Data -> ZLib -> AES -> Base64 -> Cipher
	if( Compress )
	{
		// Compress
		std::string Compressed;
		CryptoPP::StringSource(
			Data,
			true,
			new CryptoPP::ZlibCompressor(
				new CryptoPP::StringSink(Compressed)
			)
		);
		CompressedSize = Compressed.size();
		// Encrypt(AES) + Base64
		Encryptor.SetKeyWithIV(
			Key_LUT[CompressedSize % 128],
			32,
			IV_LUT[CompressedSize % 128]
		);
		CryptoPP::StringSource(
			Compressed,
			true,
			new CryptoPP::StreamTransformationFilter(
				Encryptor,
				new CryptoPP::Base64Encoder(
					new CryptoPP::StringSink(Encoded)
				)
			)
		);
	}
	else
	{
		// Encrypt(AES) + Base64
		Encryptor.SetKeyWithIV(
			Key_LUT[Data.size() % 128],
			32,
			IV_LUT[Data.size() % 128]
		);
		CryptoPP::StringSource(
			Data,
			true,
			new CryptoPP::StreamTransformationFilter(
				Encryptor,
				new CryptoPP::Base64Encoder(
					new CryptoPP::StringSink(Encoded)
				)
			)
		);
		CompressedSize = Encoded.size();
	}

	Encoded.pop_back(); // Remove trailing "\n"
	return std::make_tuple(
		Encoded,
		CompressedSize,
		Encoded.size()
	);
}

std::tuple<
	std::string, // Encrypted data
	std::uint64_t, // Compressed Size
	std::uint64_t  // Encoded Size
> EncryptFile(
	std::ifstream& FileStream,
	const std::uint8_t IV_LUT[128][16],
	const std::uint8_t Key_LUT[128][32],
	bool Compress
)
{
	std::string Encoded;
	std::uint64_t CompressedSize;

	CryptoPP::CTR_Mode<CryptoPP::AES>::Encryption Encryptor;

	// Encrypt: Data -> ZLib -> AES -> Base64 -> Cipher
	if( Compress )
	{
		// Compress
		std::string Compressed;
		CryptoPP::FileSource(
			FileStream,
			true,
			new CryptoPP::ZlibCompressor(
				new CryptoPP::StringSink(Compressed)
			)
		);
		CompressedSize = Compressed.size();
		// Encrypt(AES) + Base64
		Encryptor.SetKeyWithIV(
			Key_LUT[CompressedSize % 128],
			32,
			IV_LUT[CompressedSize % 128]
		);
		CryptoPP::StringSource(
			Compressed,
			true,
			new CryptoPP::StreamTransformationFilter(
				Encryptor,
				new CryptoPP::Base64Encoder(
					new CryptoPP::StringSink(Encoded)
				)
			)
		);
	}
	else
	{
		// Encrypt(AES) + Base64
		FileStream.seekg(0, std::ios::end);
		Encryptor.SetKeyWithIV(
			Key_LUT[FileStream.tellg() % 128],
			32,
			IV_LUT[FileStream.tellg() % 128]
		);
		FileStream.seekg(0, std::ios::beg);
		CryptoPP::FileSource(
			FileStream,
			true,
			new CryptoPP::StreamTransformationFilter(
				Encryptor,
				new CryptoPP::Base64Encoder(
					new CryptoPP::StringSink(Encoded)
				)
			)
		);
		CompressedSize = Encoded.size();
	}

	Encoded.pop_back(); // Remove trailing "\n"
	return std::make_tuple(
		Encoded,
		CompressedSize,
		Encoded.size()
	);
}

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

	// Decrypt: Cipher -> Base64 -> AES -> ZLib -> Data
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
		const fs::path HeaderFileIndex = CurFileLine.substr(
			0,
			CurFileLine.find_first_of(',')
		);
		const fs::path FileName = CurFileLine.substr(
			CurFileLine.find_last_of(',') + 1
		);

		FileListEntries[std::stoull(HeaderFileIndex)] = FileName;
	}
	return FileListEntries;
}
}
}
