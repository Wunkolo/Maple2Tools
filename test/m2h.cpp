#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <inttypes.h>
#include <iostream>
#include <vector>
#include <string>
#include <regex>
#include <map>
#include <thread>
#include <future>

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

#include <Maple2/Maple2.hpp>
#include <Util/File.hpp>

#include <cryptopp/aes.h>
#include <cryptopp/base64.h>
#include <cryptopp/modes.h>
#include <cryptopp/filters.h>
#include <cryptopp/zlib.h>

std::string DecryptStream(
	const std::string& Encoded,
	const std::uint8_t Key[32],
	const std::uint8_t IV[16],
	bool Compressed = false
);

bool DumpPackFile( const fs::path& HeaderPath, fs::path DestPath );

void HexDump( const char* Desription, const void* Data, std::size_t Size );

int main( int argc, char* argv[] )
{
	std::puts(
		"Maplestory2 PackFile tool:\n"
		"Build Date" __TIMESTAMP__ "\n"
		"\t- wunkolo <wunkolo@gmail.com>"
	);
	if( argc < 3 )
	{
		std::puts("No argument given");
		return EXIT_FAILURE;
	}

	const fs::path SourcePath(argv[1]);
	const fs::path DestPath(argv[2]);
	fs::create_directory(DestPath);

	if( !fs::exists(SourcePath))
	{
		std::puts( "Invalid source/dest paths" );
		return EXIT_FAILURE;
	}

	std::vector<std::future<bool>> Tasks;
	for( const auto& CurEntry : fs::recursive_directory_iterator( SourcePath ) )
	{
		if( fs::is_regular_file( CurEntry ) )
		{
			const fs::path CurSource = CurEntry.path();
			const fs::path CurDest = DestPath / CurEntry.path();
			fs::create_directories(CurDest.parent_path());

			// Create symlink to original files
			try
			{
				fs::create_symlink(
					fs::absolute(CurSource),
					CurDest
				);
			}
			catch( fs::filesystem_error& e)
			{
			}

			// Process Header files
			if( CurSource.extension() == ".m2h" )
			{
				const fs::path CurExpansion = CurDest.parent_path() / CurDest.stem();
				std::cout << CurSource << std::endl;
				std::cout << CurExpansion << std::endl;

				// Process .m2h into new folder of the same name
				fs::create_directory(CurExpansion);
				Tasks.emplace_back(
					std::async(DumpPackFile,CurSource,CurExpansion)
				);
			}
		}
	}

	for( auto& CurTasks : Tasks )
	{
		CurTasks.wait();
	}
	return EXIT_SUCCESS;
}

bool DumpPackFile( const fs::path& HeaderPath, fs::path DestPath)
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
	Magic = Util::Read<std::uint32_t>( FileIn );

	switch( static_cast<Maple2::Magic>( Magic ) )
	{
	case Maple2::Magic::MS2F:
	{
		break;
	}
	case Maple2::Magic::NS2F:
	case Maple2::Magic::OS2F:
	case Maple2::Magic::PS2F:
	{
		std::puts(
			"Not implemented"
		);
		return false;
		break;
	}
	}

	Maple2::PackFileHeaderVer1 CurHeader = {};
	CurHeader = Util::Read<Maple2::PackFileHeaderVer1>(FileIn);

	std::printf(
		"File: %s\n"
		"Magic: %x ( `%.4s` )\n"
		"FATCompressedSize: %zx ( %zu )\n"
		"FATEncodedSize: %zx ( %zu )\n"
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
		FileList.data(),
		CurHeader.FileListEncodedSize
	);

	/*
	   HexDump(
	   "FileList Cipher",
	   FileList.data(),
	   std::min<std::size_t>( FileList.size(), 256 )
	   );
	 */

	FileList = DecryptStream(
		FileList,
		Maple2::MS2F_Key_LUT[CurHeader.FileListCompressedSize % 128],
		Maple2::MS2F_IV_LUT[CurHeader.FileListCompressedSize % 128],
		CurHeader.FileListSize != CurHeader.FileListCompressedSize
	);

	HexDump(
		"File List",
		FileList.c_str(),
		std::min<std::size_t>( FileList.size(), 256 )
	);

	std::map<std::size_t, std::string> FileListEntries;
	{
		// Split based on \r\n
		std::regex RegExNewline("[\r\n]+");
		std::sregex_token_iterator TokenIter(FileList.begin(),FileList.end(),RegExNewline,-1);
		std::sregex_token_iterator TokenEnd;

		for( ;TokenIter != TokenEnd; ++TokenIter )
		{
			const std::string CurFileLine = (*TokenIter).str();

			const fs::path FileIndex = CurFileLine.substr( 0, CurFileLine.find_first_of(',') );
			const fs::path FileName = CurFileLine.substr( CurFileLine.find_last_of(',') + 1 );
			/*
			   std::printf(
			   "%s:\t%s\n",
			   FileIndex.c_str(),
			   FileName.c_str()
			   );
			 */
			FileListEntries[ std::stoull(FileIndex) ] = FileName;
		}
	}

	////////////////////////////////////////////////////////////////////////////
	// File allocation Table
	std::puts("Reading FAT Table");
	std::string FileAllocationTable;
	FileAllocationTable.resize(CurHeader.FATEncodedSize);
	FileIn.read(
		FileAllocationTable.data(),
		CurHeader.FATEncodedSize
	);


	HexDump(
		"FAT Cipher",
		FileAllocationTable.data(),
		std::min<std::size_t>( FileAllocationTable.size(), 256 )
	);


	FileAllocationTable = DecryptStream(
		FileAllocationTable,
		Maple2::MS2F_Key_LUT[CurHeader.FATCompressedSize % 128],
		Maple2::MS2F_IV_LUT[CurHeader.FATCompressedSize % 128],
		CurHeader.FATSize != CurHeader.FATCompressedSize
	);

	std::vector<Maple2::FATEntry> FATable;
	FATable.resize(CurHeader.TotalFiles);
	std::memcpy(
		FATable.data(),
		FileAllocationTable.data(),
		FileAllocationTable.size()
	);

	
	 HexDump(
	   "File Allocation Table",
	   FATable.data(),
	   std::min<std::size_t>( FATable.size() * sizeof(Maple2::FATEntry), 256)
	);
	


	////////////////////////////////////////////////////////////////////////////
	// Process data file
	const fs::path DataPath = fs::path(HeaderPath).replace_extension(".m2d");

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

	for( std::size_t i = 0; i < FATable.size(); ++i )
	{
		/*
		   std::printf(
		   "FileName: %s\n"
		   "FileIndex: %u\n"
		   "Offset: %zu\n"
		   "EncodedSize: %zu\n"
		   "CompressedSize: %zu\n"
		   "Size: %zu\n",
		   FileListEntries[i + 1].c_str(),
		   FATable[i].FileIndex,
		   FATable[i].Offset,
		   FATable[i].EncodedSize,
		   FATable[i].CompressedSize,
		   FATable[i].Size
		   );
		 */

		std::string FileData;
		FileData.resize( FATable[i].EncodedSize );
		DataFile.seekg( FATable[i].Offset );
		DataFile.read(
			FileData.data(),
			FATable[i].EncodedSize
		);

		/*
		   std::printf(
		   "Data Stream: %.128s...\n",
		   EncodedData.c_str()
		   );
		 */


		FileData = DecryptStream(
			FileData,
			Maple2::MS2F_Key_LUT[FATable[i].CompressedSize % 128],
			Maple2::MS2F_IV_LUT[FATable[i].CompressedSize % 128],
			FATable[i].Size != FATable[i].CompressedSize
		);

		/*
		   HexDump(
		   "PlaintextData: " ,
		   PlaintextData.data(),
		   std::min<std::size_t>( PlaintextData.size(), 256 )
		   );
		 */

		fs::create_directories(
			DestPath / fs::path( FileListEntries[ i + 1 ] ).parent_path()
		);

		std::puts( (DestPath / fs::path( FileListEntries[ i + 1 ] )).c_str() );
		std::ofstream DumpFile;
		DumpFile.open(
			DestPath / fs::path( FileListEntries[ i + 1 ] ),
			std::ios::binary
		);

		DumpFile.write(
			FileData.data(),
			FileData.size()
		);

		DumpFile.close();
	}

	return true;
}

std::string DecryptStream(
	const std::string& Encoded,
	const std::uint8_t Key[32],
	const std::uint8_t IV[16],
	bool Compressed
)
{

	std::string Decrypted;

	CryptoPP::CTR_Mode<CryptoPP::AES>::Decryption Decryptor;
	Decryptor.SetKeyWithIV( Key, 32, IV	);

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
