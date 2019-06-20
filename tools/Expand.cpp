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
#include <regex>

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

#ifdef __unix__
#include <unistd.h>
#ifdef _POSIX_VERSION
#include <pthread.h>
#endif
#endif

#include <clara.hpp>
#include <mio/mmap.hpp>

#include <Maple2/Maple2.hpp>
#include <Util/File.hpp>

bool DumpPackFile(const fs::path& HeaderPath, fs::path DestPath, std::size_t TaskIndex);

void HexDump(const char* Description, const void* Data, std::size_t Size);

int main(int argc, char* argv[])
{
	std::puts(
		"MapleStory2 Filesystem expander:\n"
		"\t\"Flattens\" a filesystem, expanding all m2h/m2d files it encounters\n"
		"\tinto a folder of the same name\n"
		"Build Date: " __TIMESTAMP__ "\n"
		"\t- wunkolo <wunkolo@gmail.com>"
	);
	std::string ShadowOption = "sym";
	// Workaround for Clara parsing an fs::path string with a space in it:
	// https://github.com/catchorg/Clara/issues/55
	std::string SourcePath;
	std::string DestPath;
	bool ShowHelp = false;
	auto CommandParser = 
		clara::Help( ShowHelp ) |
		clara::Arg( SourcePath, "Source path" )
		("Source path to expand").required() |
		clara::Arg( DestPath, "Destination path" )
		("Destination path that will contain the dumped source folder").required() |
		clara::Opt( [&]
			(std::string String)
			{
				if( std::regex_match(String,std::regex("^(none|sym|hard|copy)$")) )
				{
					ShadowOption = String;
					return clara::ParserResult::ok(clara::ParseResultType::Matched);
				}
				else
				{
					return clara::ParserResult::runtimeError("Invalid shadow option");
				}
			}, "none|sym|hard|copy")
		["-s"]["--shadow"]
		(
			"Determines how the original files will be recreated in the dump\n\n"
			"none - No shadowing\n"
			"sym  - Creates symbolic links in the dump folder(default)\n"
			"hard - Creates hard links in the dump folder\n"
			"copy - Copies the original file into the dump folder\n"
		);
	auto Result = CommandParser.parse( clara::Args( argc, argv ) );
	if( !Result )
	{
		std::printf(
			"Error parsing command line: %s\n",
			Result.errorMessage().c_str()
		);
		CommandParser.writeToStream(std::cout);
		return EXIT_FAILURE;
	}
	else if( ShowHelp || argc < 3 )
	{
		CommandParser.writeToStream(std::cout);
		return EXIT_SUCCESS;
	}
	fs::create_directory(DestPath);

	if( !fs::exists(SourcePath) || !fs::exists(DestPath) )
	{
		std::puts("Invalid source/dest paths");
		return EXIT_FAILURE;
	}

	std::vector< std::future<bool> > Tasks;
	std::size_t TaskIndex = 1;
	for( const auto& CurEntry : fs::recursive_directory_iterator(SourcePath) )
	{
		if( fs::is_regular_file(CurEntry) )
		{
			const fs::path CurEntryRelative = CurEntry.path().string().substr(
				fs::path(SourcePath).parent_path().string().length()
			);
			const fs::path& CurSource = CurEntry.path();
			const fs::path CurDest = DestPath / CurEntryRelative;

			// Create shadow original files
			try
			{
				// TODO: Enum this, or something
				// Allow overwrite
				fs::remove(CurDest);
				switch( ShadowOption[0] )
				{
				case 's': // sym
				{
					fs::create_directories(CurDest.parent_path());
					fs::create_symlink(fs::absolute(CurSource), CurDest);
					break;
				}
				case 'h': // hard
				{
					fs::create_directories(CurDest.parent_path());
					fs::create_hard_link(fs::absolute(CurSource), CurDest);
					break;
				}
				case 'c': // copy
				{
					fs::create_directories(CurDest.parent_path());
					fs::copy_file(
						fs::absolute(CurSource), CurDest,
						fs::copy_options::overwrite_existing
					);
					break;
				}
				default: // none
				{
					break;
				}
				}
			}
			catch( fs::filesystem_error& Exception)
			{
				std::printf(
					"Failed to create shadow file (%s):\n%s\n",
					CurSource.c_str(), Exception.what()
				);
				continue;
			}

			// Process Header files
			if( CurSource.extension() == ".m2h" )
			{
				const fs::path CurExpansion = CurDest.parent_path() / CurDest.stem();

				fs::create_directories(CurDest.parent_path());
				// Process .m2h into new folder of the same name
				fs::create_directory(CurExpansion);
				const auto ThreadProc = []
				(
					const fs::path& Source, const fs::path& Expansion,
					std::size_t TaskIndex
				) -> bool
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
					return DumpPackFile(Source,Expansion, TaskIndex);
				};
				Tasks.emplace_back(
					std::async(ThreadProc, CurSource, CurExpansion, TaskIndex++)
				);
			}
		}
	}

	for( auto& CurTasks : Tasks ) CurTasks.get();
	std::printf("\033[%zuB\n", TaskIndex + 1);
	return EXIT_SUCCESS;
}

template< typename PackTraits >
bool DumpPackStream(const fs::path& HeaderPath, fs::path DestPath,std::size_t TaskIndex)
{
	mio::ummap_source HeaderFile(HeaderPath.c_str(), 0, mio::map_entire_file);
	if( !HeaderFile.is_open() )
	{
		// Error opening file
		std::printf(
			"Error opening file for reading: %s\n",
			HeaderPath.string().c_str()
		);
		return false;
	}

	auto HeaderReadPoint = HeaderFile.cbegin();

	const Maple2::Identifier Magic
		 = *reinterpret_cast<const typename Maple2::Identifier*>(HeaderReadPoint);
	HeaderReadPoint += sizeof(Maple2::Identifier);

	if( Magic != PackTraits::Magic )
	{
		// Invalid magic
		return false;
	}

	std::printf(
		"\033[%zuB"                                             // Move Down
		"\033[2K"                                               // Clear line
		"\r"                                                    // Return to left
		"[ %-20.20s ] | \033[0;33mParsing file list... \033[0m" // Printed string 
		"\033[%zuA",                                            // Move up
		TaskIndex,
		HeaderPath.stem().c_str(),
		TaskIndex
	);
	typename PackTraits::StreamType StreamHeader
		= *reinterpret_cast<const typename PackTraits::StreamType*>(HeaderReadPoint);
	HeaderReadPoint += sizeof(typename PackTraits::StreamType);

	////////////////////////////////////////////////////////////////////////////
	// FileList
	std::string FileList;
	FileList.resize(StreamHeader.FileListSize);
	Maple2::Util::DecryptStream(
		HeaderReadPoint,
		StreamHeader.FileListEncodedSize,
		PackTraits::IV_LUT[StreamHeader.FileListCompressedSize % 128],
		PackTraits::Key_LUT[StreamHeader.FileListCompressedSize % 128],
		FileList.data(),
		StreamHeader.FileListSize,
		StreamHeader.FileListSize != StreamHeader.FileListCompressedSize
	);

	HeaderReadPoint += StreamHeader.FileListEncodedSize;

	// Generate list of File list entries
	const std::map<std::size_t, fs::path> FileListEntries
		= Maple2::Util::ParseFileList(FileList);

	////////////////////////////////////////////////////////////////////////////
	// File allocation Table

	std::vector<typename PackTraits::FileHeaderType> FATable;
	FATable.resize(StreamHeader.TotalFiles, typename PackTraits::FileHeaderType{});
	Maple2::Util::DecryptStream(
		HeaderReadPoint,
		StreamHeader.FATEncodedSize,
		PackTraits::IV_LUT[StreamHeader.FATCompressedSize % 128],
		PackTraits::Key_LUT[StreamHeader.FATCompressedSize % 128],
		FATable.data(),
		StreamHeader.TotalFiles * sizeof(typename PackTraits::FileHeaderType),
		StreamHeader.FATSize != StreamHeader.FATCompressedSize
	);

	HeaderFile.unmap();

	////////////////////////////////////////////////////////////////////////////
	// Process data file
	const fs::path DataPath = fs::path(HeaderPath).replace_extension(".m2d");

	mio::ummap_source DataFile(DataPath.c_str(), 0, mio::map_entire_file);

	if( !DataFile.is_open() )
	{
		// Error opening file
		std::printf(
			"Error opening file for reading: %s\n", DataPath.string().c_str()
		);
		return false;
	}
	for( std::size_t i = 0; i < StreamHeader.TotalFiles; ++i )
	{
		fs::create_directories(
			DestPath / FileListEntries.at(i + 1).parent_path()
		);

		std::printf(
			"\033[%zuB"                                                           // Move Down
			"\033[2K"                                                             // Clear line
			"\r"                                                                  // Return to left
			"[ %-20.20s ] | \033[1;37m%6.2f%%\033[0m \033[1;34m%-60.60s\033[0m"   // Printed string 
			"\033[%zuA",                                                          // Move up
			TaskIndex,
			HeaderPath.stem().string().c_str(),                                   // PackFile
			(static_cast<float>(i + 1) / FATable.size()) * 100.0f,                // Percentage
			fs::path(FileListEntries.at(i + 1)).string().c_str(),                 // Current file
			TaskIndex
		);

		std::ofstream DumpFile;
		DumpFile.open(
			DestPath / FileListEntries.at(i + 1), std::ios::binary
		);
		if( !DumpFile )
		{
			std::printf(
				"Error opening file \"%s\" for writing\n",
				(DestPath / FileListEntries.at(i + 1)).c_str()
			);
			return false;
		}
		Maple2::Util::DecryptStreamToStream(
			DataFile.cbegin() + FATable[i].Offset,
			FATable[i].EncodedSize,
			PackTraits::IV_LUT[FATable[i].CompressedSize % 128],
			PackTraits::Key_LUT[FATable[i].CompressedSize % 128],
			DumpFile,
			FATable[i].Size != FATable[i].CompressedSize
		);
		DumpFile.close();
	}
	DataFile.unmap();

	return true;
}

bool DumpPackFile(const fs::path& HeaderPath, fs::path DestPath, std::size_t TaskIndex)
{
	std::ifstream FileIn;
	FileIn.open(HeaderPath, std::ios::binary);

	if( !FileIn.good() )
	{
		// Error opening file
		std::printf(
			"Error opening file for reading: %s\n",
			HeaderPath.string().c_str()
		);
		return false;
	}

	const Maple2::Identifier Magic = Util::Read<Maple2::Identifier>(FileIn);
	FileIn.close();

	bool Result = false;
	try
	{
		switch( Magic )
		{
		case Maple2::Identifier::MS2F:
		{
			Result = DumpPackStream<Maple2::PackTraits::MS2F>(
				HeaderPath, DestPath, TaskIndex
			);
			break;
		}
		case Maple2::Identifier::NS2F:
		{
			Result = DumpPackStream<Maple2::PackTraits::NS2F>(
				HeaderPath, DestPath, TaskIndex
			);
			break;
		}
		case Maple2::Identifier::OS2F:
		{
			Result = DumpPackStream<Maple2::PackTraits::OS2F>(
				HeaderPath, DestPath, TaskIndex
			);
			break;
		}
		case Maple2::Identifier::PS2F:
		{
			Result = DumpPackStream<Maple2::PackTraits::PS2F>(
				HeaderPath, DestPath, TaskIndex
			);
			break;
		}
		}
	}
	catch(const std::exception& Exception)
	{
		std::printf(
			"\033[%zuB"                                      // Move Down
			"\033[2K"                                        // Clear line
			"\r"                                             // Return to left
			"[ %-20.20s ] | \033[0;31mERROR: %s\033[0m"      // Printed string 
			"\033[%zuA",                                     // Move up
			TaskIndex,
			HeaderPath.stem().string().c_str(),
			Exception.what(),
			TaskIndex
		);
		return false;
	}

	std::printf(
		"\033[%zuB"                               // Move Down
		"\033[2K"                                 // Clear line
		"\r"                                      // Return to left
		"[ %-20.20s ] | \033[0;32mDONE \033[0m"   // Printed string 
		"\033[%zuA",                              // Move up
		TaskIndex,
		HeaderPath.stem().string().c_str(),
		TaskIndex
	);
	return Result;
}

void HexDump(const char* Description, const void* Data, std::size_t Size)
{
	std::size_t i;
	std::uint8_t Buffer[17];
	const std::uint8_t* CurByte = reinterpret_cast<const std::uint8_t*>(Data);

	if( Description != nullptr )
	{
		std::printf("\e[5m%s\e[0m:\n", Description);
	}

	for( i = 0; i < Size; i++ )
	{
		if( (i % 16) == 0 )
		{
			if( i != 0 )
			{
				std::printf("  \e[0;35m%s\e[0m\n", Buffer);
			}

			std::printf("  \e[0;33m%04zx\e[0m ", i);
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

	std::printf("  \e[0;35m%s\e[0m\n", Buffer);
}
