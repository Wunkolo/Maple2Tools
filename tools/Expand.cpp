#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <cinttypes>
#include <iostream>
#include <vector>
#include <string>
#include <regex>
#include <map>
#include <thread>
#include <future>

#ifdef __unix__
#include <unistd.h>
#ifdef _POSIX_VERSION
#include <pthread.h>
#endif
#endif

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

#include <Maple2/Maple2.hpp>
#include <Util/File.hpp>

bool DumpPackFile(const fs::path& HeaderPath, fs::path DestPath);

void HexDump(const char* Description, const void* Data, std::size_t Size);

int main(int argc, char* argv[])
{
	std::puts(
		"MapleStory2 Filesystem expander:\n"
		"\t\"Flattens\" a filesystem, expanding all m2h/m2d files it encounters\n"
		"\tinto a folder of the same name\n"
		"Build Date: " __TIMESTAMP__ "\n"
		"\t- wunkolo <wunkolo@gmail.com>\n"
		"Usage: Expand (Source) (Dest)\n"
	);
	if( argc < 3 )
	{
		std::puts("No argument given");
		return EXIT_FAILURE;
	}

	const fs::path SourcePath(argv[1]);
	const fs::path DestPath(argv[2]);
	fs::create_directory(DestPath);

	if( !fs::exists(SourcePath) )
	{
		std::puts("Invalid source/dest paths");
		return EXIT_FAILURE;
	}

	std::vector<std::future<bool> > Tasks;
	for( const auto& CurEntry : fs::recursive_directory_iterator(SourcePath) )
	{
		if( fs::is_regular_file(CurEntry) )
		{
			const fs::path& CurSource = CurEntry.path();
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
			catch( fs::filesystem_error& )
			{
			}

			// Process Header files
			if( CurSource.extension() == ".m2h" )
			{
				const fs::path CurExpansion = CurDest.parent_path() / CurDest.stem();
				// std::cout << CurSource << std::endl;
				// std::cout << CurExpansion << std::endl;

				// Process .m2h into new folder of the same name
				fs::create_directory(CurExpansion);
				const auto ThreadProc = []
				(const fs::path& Source, const fs::path& Expansion) -> bool
				{
				#ifdef _POSIX_VERSION
					char ThreadName[16] = {0};
					const auto PackName = Source.stem().string();
					std::sprintf(
						ThreadName,
						PackName.length() > 7 ? "Expand: %.4s...":"Expand: %.7s",
						PackName.c_str()
					);

					pthread_setname_np(pthread_self(), ThreadName);
				#endif
					return DumpPackFile(Source,Expansion);
				};
				Tasks.emplace_back(
					std::async(ThreadProc, CurSource, CurExpansion)
				);
			}
		}
	}

	for( auto& CurTasks : Tasks )
	{
		CurTasks.get();
	}
	return EXIT_SUCCESS;
}

template< typename PackTraits >
bool DumpPackStream(const fs::path& HeaderPath, fs::path DestPath)
{
	std::ifstream HeaderFile;
	HeaderFile.open(
		HeaderPath,
		std::ios::binary
	);

	if( !HeaderFile.good() )
	{
		// Error opening file
		std::printf(
			"Error opening file for reading: %ls\n",
			HeaderPath.wstring().c_str()
		);
		return false;
	}

	const Maple2::Identifier Magic = Util::Read<Maple2::Identifier>(HeaderFile);
	if( Magic != PackTraits::Magic )
	{
		// Invalid magic
		return false;
	}

	typename PackTraits::StreamType StreamHeader = {};
	StreamHeader = Util::Read<typename PackTraits::StreamType>(HeaderFile);

	std::printf(
		"File: %ls\n"
		"Magic: %x ( `%.4s` )\n"
		"FATCompressedSize: %zx ( %zu )\n"
		"FATEncodedSize: %zx ( %zu )\n"
		"FileListSize: %zx ( %zu )\n"
		"FileListCompressedSize: %zx ( %zu )\n"
		"FileListEncodedSize: %zx ( %zu )\n"
		"TotalFiles: %zx ( %zu )\n"
		"FATSize: %zx ( %zu )\n"
		"\n",
		HeaderPath.wstring().c_str(),
		static_cast<std::uint32_t>(Magic),
		reinterpret_cast<const char*>(&Magic),
		static_cast<std::size_t>(StreamHeader.FATCompressedSize),
		static_cast<std::size_t>(StreamHeader.FATCompressedSize),
		static_cast<std::size_t>(StreamHeader.FATEncodedSize),
		static_cast<std::size_t>(StreamHeader.FATEncodedSize),
		static_cast<std::size_t>(StreamHeader.FileListSize),
		static_cast<std::size_t>(StreamHeader.FileListSize),
		static_cast<std::size_t>(StreamHeader.FileListCompressedSize),
		static_cast<std::size_t>(StreamHeader.FileListCompressedSize),
		static_cast<std::size_t>(StreamHeader.FileListEncodedSize),
		static_cast<std::size_t>(StreamHeader.FileListEncodedSize),
		static_cast<std::size_t>(StreamHeader.TotalFiles),
		static_cast<std::size_t>(StreamHeader.TotalFiles),
		static_cast<std::size_t>(StreamHeader.FATSize),
		static_cast<std::size_t>(StreamHeader.FATSize)
	);

	////////////////////////////////////////////////////////////////////////////
	// FileList
	std::string FileList;
	FileList.resize(StreamHeader.FileListEncodedSize);
	HeaderFile.seekg(
		4 + sizeof(typename PackTraits::StreamType),
		std::ios::beg
	);
	HeaderFile.read(
		FileList.data(),
		StreamHeader.FileListEncodedSize
	);

	// HexDump(
	// 	"FileList Cipher",
	// 	FileList.data(),
	// 	std::min<std::size_t>( FileList.size(), 256 )
	// );

	FileList = Maple2::Util::DecryptStream(
		FileList,
		PackTraits::IV_LUT[StreamHeader.FileListCompressedSize % 128],
		PackTraits::Key_LUT[StreamHeader.FileListCompressedSize % 128],
		StreamHeader.FileListSize != StreamHeader.FileListCompressedSize
	);

	// HexDump(
	// 	"File List",
	// 	FileList.c_str(),
	// 	std::min<std::size_t>( FileList.size(), 256 )
	// );

	// Generate list of File list entries
	const std::map<std::size_t, fs::path> FileListEntries = Maple2::Util::ParseFileList(FileList);

	////////////////////////////////////////////////////////////////////////////
	// File allocation Table
	std::puts("Reading FAT Table");
	std::string FileAllocationTable;
	FileAllocationTable.resize(StreamHeader.FATEncodedSize);
	HeaderFile.read(
		FileAllocationTable.data(),
		StreamHeader.FATEncodedSize
	);

	// HexDump(
	// 	"FAT Cipher",
	// 	FileAllocationTable.data(),
	// 	std::min<std::size_t>( FileAllocationTable.size(), 256 )
	// );

	FileAllocationTable = Maple2::Util::DecryptStream(
		FileAllocationTable,
		PackTraits::IV_LUT[StreamHeader.FATCompressedSize % 128],
		PackTraits::Key_LUT[StreamHeader.FATCompressedSize % 128],
		StreamHeader.FATSize != StreamHeader.FATCompressedSize
	);

	std::vector<typename PackTraits::FileHeaderType> FATable;
	FATable.resize(StreamHeader.TotalFiles);
	std::memcpy(
		FATable.data(),
		FileAllocationTable.data(),
		FileAllocationTable.size()
	);

	// HexDump(
	// 	"File Allocation Table",
	// 	FATable.data(),
	// 	std::min<std::size_t>(
	// 		FATable.size() * sizeof(typename PackTraits::FileHeaderType),
	// 		256
	// 	)
	// );

	HeaderFile.close();

	////////////////////////////////////////////////////////////////////////////
	// Process data file
	const fs::path DataPath = fs::path(HeaderPath).replace_extension(".m2d");

	std::printf(
		"Processing data file: %ls\n",
		DataPath.wstring().c_str()
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
			"Error opening file for reading: %ls\n",
			DataPath.wstring().c_str()
		);
		return false;
	}

	for( std::size_t i = 0; i < FATable.size(); ++i )
	{
		// std::printf(
		// 	"FileName: %s\n"
		// 	"FileIndex: %u\n"
		// 	"Offset: %zu\n"
		// 	"EncodedSize: %u\n"
		// 	"CompressedSize: %zu\n"
		// 	"Size: %zu\n",
		// 	FileListEntries[i + 1].c_str(),
		// 	FATable[i].FileIndex,
		// 	FATable[i].Offset,
		// 	FATable[i].EncodedSize,
		// 	FATable[i].CompressedSize,
		// 	FATable[i].Size
		// );

		std::string FileData;
		FileData.resize(FATable[i].EncodedSize);
		DataFile.seekg(FATable[i].Offset);
		DataFile.read(
			FileData.data(),
			FATable[i].EncodedSize
		);

		// std::printf(
		// 	"Data Stream: %.128s...\n",
		// 	FileData.c_str()
		// );

		FileData = Maple2::Util::DecryptStream(
			FileData,
			PackTraits::IV_LUT[FATable[i].CompressedSize % 128],
			PackTraits::Key_LUT[FATable[i].CompressedSize % 128],
			FATable[i].Size != FATable[i].CompressedSize
		);

		// HexDump(
		// 	"PlaintextData: " ,
		// 	FileData.data(),
		// 	std::min<std::size_t>( FileData.size(), 256 )
		// );

		fs::create_directories(
			DestPath / FileListEntries.at(i + 1).parent_path()
		);

		std::printf(
			"[ %ls ]: %ls\n",
			HeaderPath.stem().wstring().c_str(),
			(
				DestPath / fs::path(FileListEntries.at(i + 1))
			).wstring().c_str()
		);
		std::ofstream DumpFile;
		DumpFile.open(
			DestPath / FileListEntries.at(i + 1),
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

bool DumpPackFile(const fs::path& HeaderPath, fs::path DestPath)
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
			"Error opening file for reading: %ls\n",
			HeaderPath.wstring().c_str()
		);
		return false;
	}

	const Maple2::Identifier Magic = Util::Read<Maple2::Identifier>(FileIn);
	FileIn.close();

	switch( Magic )
	{
	case Maple2::Identifier::MS2F:
	{
		return DumpPackStream<Maple2::PackTraits::MS2F>(
			HeaderPath,
			DestPath
		);
	}
	case Maple2::Identifier::NS2F:
	{
		return DumpPackStream<Maple2::PackTraits::NS2F>(
			HeaderPath,
			DestPath
		);
	}
	case Maple2::Identifier::OS2F:
	{
		return DumpPackStream<Maple2::PackTraits::OS2F>(
			HeaderPath,
			DestPath
		);
	}
	case Maple2::Identifier::PS2F:
	{
		return DumpPackStream<Maple2::PackTraits::PS2F>(
			HeaderPath,
			DestPath
		);
	}
	}

	return false;
}

void HexDump(const char* Description, const void* Data, std::size_t Size)
{
	std::size_t i;
	std::uint8_t Buffer[17];
	const std::uint8_t* CurByte = reinterpret_cast<const std::uint8_t*>(Data);

	if( Description != nullptr )
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
			if( i != 0 )
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
