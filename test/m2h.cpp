#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <inttypes.h>
#include <iostream>
#include <vector>

#include <Maple2/Maple2.hpp>
#include <Util/File.hpp>

#include <cryptopp/aes.h>
#include <cryptopp/base64.h>
#include <cryptopp/modes.h>
#include <cryptopp/filters.h>
#include <cryptopp/zinflate.h>

#include <zlib.h>

std::string Decrypt( const std::string& Cipher , std::size_t KeyIndex )
{
	std::string Decrypted;

	CryptoPP::CTR_Mode<CryptoPP::AES>::Encryption Decryptor;
	Decryptor.SetKeyWithIV(
		Maple2::MS2F_Key_LUT[KeyIndex % 128],
		32,
		Maple2::MS2F_IV_LUT[KeyIndex % 128]
	);

	CryptoPP::StringSource(
		Cipher,
		true,
		new CryptoPP::Base64Decoder(
			new CryptoPP::StreamTransformationFilter(
				Decryptor,
				new CryptoPP::StringSink(Decrypted)
			)
		)
	);

	std::string Decompressed;
	Decompressed.resize(Cipher.size() * 8);

	std::uint64_t DecompressedSize;
	uncompress(
		reinterpret_cast<std::uint8_t*>(&Decompressed[0]),
		&DecompressedSize,
		reinterpret_cast<const std::uint8_t*>(Decrypted.c_str()),
		Decrypted.size()
	);
	Decompressed.resize(DecompressedSize);
	return Decompressed;
}


void HexDump( const char* Desription, const void* Data, std::size_t Size);

bool ProcessFile( const std::string& HeaderPath );

int main( int argc, char* argv[] )
{
	std::puts(
		"Maplestory2 PackFile tool:\n"
		"Build Date" __TIMESTAMP__ "\n"
		"\t- wunkolo <wunkolo@gmail.com>"
	);
	if( argc < 2 )
	{
		std::puts("No argument given");
		return EXIT_FAILURE;
	}

	for( std::size_t i = 1; i < static_cast<std::size_t>(argc); ++i )
	{
		ProcessFile(argv[i]);
	}

	return EXIT_SUCCESS;
}

bool ProcessFile( const std::string& HeaderPath )
{
	std::ifstream FileIn;
	FileIn.open(
		HeaderPath,
		std::ios::binary
	);

	if( !FileIn.good() )
	{
		// Error opening file
		std::printf(
			"Error opening file for reading: %s\n",
			HeaderPath.c_str()
		);
		return false;
	}

	std::uint32_t Magic = 0;
	Magic = Util::Read<std::uint32_t>(FileIn);

	switch( static_cast<Maple2::Magic>(Magic) )
	{
	case Maple2::Magic::MS2F:
	{
		break;
	}
	case Maple2::Magic::NS2F:
	{
		break;
	}
	case Maple2::Magic::OS2F:
	{
		break;
	}
	case Maple2::Magic::PS2F:
	{
		break;
	}
	}

	Maple2::PackFileHeaderVer1 CurHeader = {};
	CurHeader = Util::Read<Maple2::PackFileHeaderVer1>(FileIn);

	std::printf(
		"File: %s\n"
		"Magic: %x ( `%.4s` )\n"
		"FATCompressedSize: %zx ( %zu )\n"
		"FATEncodedSize: %zx ( %zu ) Size\n"
		"FileListSize: %zx ( %zu )\n"
		"FileListCompressedSize: %zx ( %zu )\n"
		"FileListEncodedSize: %zx ( %zu )\n"
		"TotalFiles: %zx ( %zu )\n"
		"FATSize: %zx ( %zu )\n"
		"\n"
		,
		HeaderPath.c_str(),
		Magic, reinterpret_cast<const char*>(&Magic),
		CurHeader.FATCompressedSize, CurHeader.FATCompressedSize,
		CurHeader.FATEncodedSize, CurHeader.FATEncodedSize,
		CurHeader.FileListSize, CurHeader.FileListSize,
		CurHeader.FileListCompressedSize, CurHeader.FileListCompressedSize,
		CurHeader.FileListEncodedSize, CurHeader.FileListEncodedSize,
		CurHeader.TotalFiles, CurHeader.TotalFiles,
		CurHeader.FATSize, CurHeader.FATSize
	);

	////////////////////////////////////////////////////////////////////////////
	// FileList
	std::string FileList;
	FileList.resize(CurHeader.FileListEncodedSize);
	FileIn.read(
		&FileList[0],
		CurHeader.FileListEncodedSize
	);

	HexDump(
		"FileList Cipher",
		FileList.data(),
		std::min<std::size_t>( FileList.size(), 256 )
	);

	std::string DecryptedFL;

	CryptoPP::CTR_Mode<CryptoPP::AES>::Decryption DecryptorFL;
	DecryptorFL.SetKeyWithIV(
		Maple2::MS2F_Key_LUT[CurHeader.FileListCompressedSize % 128],
		32,
		Maple2::MS2F_IV_LUT[CurHeader.FileListCompressedSize % 128]
	);
	CryptoPP::StringSource(
		FileList,
		true,
		new CryptoPP::Base64Decoder(
			new CryptoPP::StreamTransformationFilter(
				DecryptorFL,
				new CryptoPP::StringSink(DecryptedFL)
			)
		)
	);

	std::string DecompressedFL;
	DecompressedFL.resize(CurHeader.FileListSize);
	std::uint64_t DecompressedFLSize;
	uncompress(
		reinterpret_cast<std::uint8_t*>(&DecompressedFL[0]),
		&DecompressedFLSize,
		reinterpret_cast<const std::uint8_t*>(DecryptedFL.c_str()),
		DecryptedFL.size()
	);
	FileList = DecompressedFL;

	HexDump(
		"File List",
		FileList.data(),
		std::min<std::size_t>( FileList.size(), 256 )
	);

	std::printf(
		"------------------------------------------------\n"
		"%s\n"
		"------------------------------------------------\n",
		FileList.c_str()
	);


	////////////////////////////////////////////////////////////////////////////
	// File allocation Table
	std::string FileAllocationTable;
	FileAllocationTable.resize(CurHeader.FATEncodedSize);
	FileIn.read(
		&FileAllocationTable[0],
		CurHeader.FATEncodedSize
	);

	HexDump(
		"FAT Cipher",
		FileAllocationTable.data(),
		std::min<std::size_t>( FileAllocationTable.size(), 256 )
	);

	CryptoPP::CTR_Mode<CryptoPP::AES>::Decryption DecryptorTOC;
	DecryptorTOC.SetKeyWithIV(
		Maple2::MS2F_Key_LUT[CurHeader.FATCompressedSize % 128],
		32,
		Maple2::MS2F_IV_LUT[CurHeader.FATCompressedSize % 128]
	);

	std::string DecryptedTOC;
	CryptoPP::StringSource(
		FileAllocationTable,
		true,
		new CryptoPP::Base64Decoder(
			new CryptoPP::StreamTransformationFilter(
				DecryptorTOC,
				new CryptoPP::StringSink(DecryptedTOC)
			)
		)
	);

	std::vector<Maple2::FATEntry> FATable;
	FATable.resize(CurHeader.TotalFiles);
	std::uint64_t DecompressedTOCSize;
	uncompress(
		reinterpret_cast<std::uint8_t*>(FATable.data()),
		&DecompressedTOCSize,
		reinterpret_cast<const std::uint8_t*>(DecryptedTOC.c_str()),
		DecryptedTOC.size()
	);

	HexDump(
		"File Allocation Table",
		FATable.data(),
		std::min<std::size_t>( FATable.size() * sizeof(Maple2::FATEntry), 256)
	);

	for( std::size_t i = 0; i < FATable.size() % 256; ++i )
	{
		std::printf(
			"FileIndex: %u\n"
			"Offset: %zu\n"
			"EncodedSize: %zu\n"
			"CompressedSize: %zu\n"
			"Size: %zu\n",
			FATable[i].FileIndex,
			FATable[i].Offset,
			FATable[i].EncodedSize,
			FATable[i].CompressedSize,
			FATable[i].Size
		);
	}

	////////////////////////////////////////////////////////////////////////////
	// Process data file
	const std::string DataPath = HeaderPath.substr(
		0,
		HeaderPath.find_last_of('.')
	) + ".m2d";

	std::printf(
		"Processing data file: %s\n",
		DataPath.c_str()
	);

	std::ifstream DataFile;
	DataFile.open(
		DataPath,
		std::ios::binary
	);

	if( !DataFile.good() )
	{
		// Error opening file
		std::printf(
			"Error opening file for reading: %s\n",
			DataPath.c_str()
		);
		return false;
	}

	CryptoPP::CTR_Mode<CryptoPP::AES>::Decryption DataDecryptor;
	DataDecryptor.SetKeyWithIV(
		Maple2::MS2F_Key_LUT[CurHeader.FATCompressedSize % 128],
		32,
		Maple2::MS2F_IV_LUT[CurHeader.FATCompressedSize % 128]
	);

	std::string EncodedData;
	DataFile >> EncodedData;
	std::printf(
		"Data Stream: %.128s...\n",
		EncodedData.c_str()
	);
	std::puts("Decrypting");
	std::string DecryptedData;
	CryptoPP::StringSource(
		EncodedData,
		true,
		new CryptoPP::Base64Decoder(
			new CryptoPP::StreamTransformationFilter(
				DataDecryptor,
				new CryptoPP::StringSink(DecryptedData)
			)
		)
	);

	HexDump(
		"DecryptedData: " ,
		DecryptedData.data(),
		std::min<std::size_t>( DecryptedData.size(), 256 )
	);
	return true;
}

void HexDump( const char* Description, const void* Data, std::size_t Size )
{
	std::size_t i;
	std::uint8_t Buffer[17];
	const std::uint8_t* CurByte = reinterpret_cast<const std::uint8_t*>(Data);

	if( Description != NULL)
	{
		std::printf(
			"\e[5m%s\e[0m:\n",
			Description
		);
	}

	for( i = 0; i < Size; i++ )
	{
		if( (i % 16) == 0 )
		{
			if( i != 0)
			{
				std::printf("  \e[0;35m%s\e[0m\n", Buffer);
			}

			std::printf(
				"  \e[0;33m%04zx\e[0m ",
				i
			);
		}
		std::printf(
			" \e[0;36m%02x\e[0m",
			CurByte[i]
		);
		if( (CurByte[i] < ' ') || (CurByte[i] > 0x7e) )
		{
			Buffer[i % 16] = '.';
		}
		else
		{
			Buffer[i % 16] = CurByte[i];
		}
		Buffer[(i % 16) + 1] = '\0';
	}

	while( (i % 16) != 0 )
	{
		std::printf("   ");
		i++;
	}

	std::printf(
		"  \e[0;35m%s\e[0m\n",
		Buffer
	);
}
