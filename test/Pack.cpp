#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <vector>
#include <sstream>
#include <cstring>

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

#include <Maple2/Maple2.hpp>

#include <Util/Util.hpp>

bool PackFolder(
	const fs::path& TargetFolder,
	Maple2::Identifier PackVersion
);

int main( int argc, char* argv[] )
{
	std::puts(
		"MapleStory2 Filesystem packer:\n"
		"\t\"Packs\" a filesystem, creating an .m2h/.m2d pair\n"
		"\tof the original folder's name\n"
		"Build Date: " __TIMESTAMP__ "\n"
		"\t- wunkolo <wunkolo@gmail.com>\n"
		"Usage: Pack (M2SF/N2SF/O2SF/P2SF) (List of folders to pack)\n"
	);
	if( argc < 2 )
	{
		std::puts("No argument given");
		return EXIT_FAILURE;
	}

	for( std::size_t i = 2; i < static_cast<std::size_t>(argc); ++i )
	{
		switch( *reinterpret_cast<Maple2::Identifier*>(argv[1]) )
		{
		case Maple2::Identifier::MS2F:
		{
			PackFolder( argv[i], Maple2::Identifier::MS2F );
			break;
		}
		case Maple2::Identifier::NS2F:
		{
			PackFolder( argv[i], Maple2::Identifier::NS2F );
			break;
		}
		case Maple2::Identifier::OS2F:
		{
			PackFolder( argv[i], Maple2::Identifier::OS2F );
			break;
		}
		case Maple2::Identifier::PS2F:
		{
			PackFolder( argv[i], Maple2::Identifier::PS2F );
			break;
		}
		default:
		{
			std::printf(
				"Unknown PackStream version: \'%s\'",
				argv[1]
			);
			return EXIT_FAILURE;
		}
		}
	}

	return EXIT_SUCCESS;
}

template< typename PackTraits >
bool MakePackFile(
	const fs::path& TargetFolder,
	std::ofstream& HeaderFile,
	std::ofstream& DataFile
)
{
	typename PackTraits::StreamType StreamHeader = {};
	std::vector<typename PackTraits::FileHeaderType> FileTable;
	std::ostringstream FileList;

	for( 
		const auto& CurFile
		: fs::recursive_directory_iterator(
			TargetFolder,
			fs::directory_options::follow_directory_symlink
		)
	)
	{
		const auto& CurPath = CurFile.path();
		if( !fs::is_regular_file( CurPath ) )
		{
			// Skip non-files
			continue;
		}
		StreamHeader.TotalFiles++;
		const fs::path RelativePath = fs::path(
			CurPath.wstring().substr(
				TargetFolder.wstring().size() + 1
			)
		);

		// Create Encoded data
		std::ifstream CurFileStream(
			CurPath,
			std::ios::binary
		);
		std::string Encoded;

		typename PackTraits::FileHeaderType CurFATEntry = {};
		std::tie(
			Encoded,
			CurFATEntry.CompressedSize, // DEFLATE length
			CurFATEntry.EncodedSize     // Base64 length
		) = Maple2::Util::EncryptFile(
			CurFileStream,
			PackTraits::IV_LUT,
			PackTraits::Key_LUT,
			true
		);
		CurFileStream.close();

		// Create FAT Entry
		CurFATEntry.Offset = static_cast<std::size_t>(DataFile.tellp());
		CurFATEntry.FileIndex = StreamHeader.TotalFiles;
		CurFATEntry.Compression = Maple2::CompressionType::Deflate;
		CurFATEntry.Size = fs::file_size( CurPath );
		FileTable.push_back(CurFATEntry);

		// Write Encoded data
		DataFile << Encoded;

		// Add to FileList
		FileList
			<< StreamHeader.TotalFiles
			<< ','
			<< RelativePath.string()
			<< "\r\n";
	}

	// Create encrypted filelist
	const std::string FileListData = FileList.str();
	std::puts(
		FileListData.c_str()
	);
	std::string FileListCipher;
	StreamHeader.FileListSize = FileListData.size();
	std::tie(
		FileListCipher,
		StreamHeader.FileListCompressedSize, // DEFLATE length
		StreamHeader.FileListEncodedSize     // Base64 length
	) = Maple2::Util::EncryptString(
		FileListData,
		PackTraits::IV_LUT,
		PackTraits::Key_LUT,
		true
	);

	// Create encrypted File Allocation Table
	std::string FATString;
	std::string FATCipher;
	FATString.resize(
		FileTable.size() * sizeof(typename PackTraits::FileHeaderType)
	);
	std::memcpy(
		FATString.data(),
		FileTable.data(),
		FileTable.size() * sizeof(typename PackTraits::FileHeaderType)
	);
	StreamHeader.FATSize = FATString.size();
	std::tie(
		FATCipher,
		StreamHeader.FATCompressedSize,      // DEFLATE length
		StreamHeader.FATEncodedSize          // Base64 length
	) = Maple2::Util::EncryptString(
		FATString,
		PackTraits::IV_LUT,
		PackTraits::Key_LUT,
		true
	);

	// Write header
	Util::Write(HeaderFile, PackTraits::Magic);
	Util::Write(HeaderFile, StreamHeader);

	// Write File List
	HeaderFile << FileListCipher;

	// Write File Allocation Table
	HeaderFile << FATCipher;

	return true;
}

bool PackFolder(
	const fs::path& TargetFolder,
	Maple2::Identifier PackVersion
)
{
	if( !fs::exists( TargetFolder ) )
	{
		std::printf(
			"Folder \"%ls\" does not exist\n",
			TargetFolder.wstring().c_str()
		);
		return false;
	}

	if( !fs::is_directory( TargetFolder ) )
	{
		std::printf(
			"\"%ls\" is not a folder\n",
			TargetFolder.wstring().c_str()
		);
		return false;
	}

	auto TargetFile = fs::absolute(TargetFolder).parent_path() / TargetFolder.stem();

	std::ofstream HeaderFile;
	HeaderFile.open(
		TargetFile.replace_extension(L".m2h"),
		std::ios::binary | std::ios::trunc
	);
	std::printf(
		"Creating header file: %ls\n",
		TargetFile.wstring().c_str()
	);
	if( !HeaderFile.good() )
	{
		std::printf(
			"Error creating file: %ls\n",
			TargetFile.wstring().c_str()
		);
		return false;
	}


	std::ofstream DataFile;
	DataFile.open(
		TargetFile.replace_extension(L".m2d"),
		std::ios::binary | std::ios::trunc
	);
	std::printf(
		"Creating data file: %ls\n",
		TargetFile.wstring().c_str()
	);
	if( !DataFile.good() )
	{
		std::printf(
			"Error creating file: %ls\n",
			TargetFile.wstring().c_str()
		);
		return false;
	}

	switch( PackVersion )
	{
	case Maple2::Identifier::MS2F:
	{
		MakePackFile<Maple2::PackTraits::MS2F>(
			TargetFolder,
			HeaderFile,
			DataFile
		);
		break;
	}
	case Maple2::Identifier::NS2F:
	{
		MakePackFile<Maple2::PackTraits::NS2F>(
			TargetFolder,
			HeaderFile,
			DataFile
		);
		break;
	}
	case Maple2::Identifier::OS2F:
	{
		MakePackFile<Maple2::PackTraits::OS2F>(
			TargetFolder,
			HeaderFile,
			DataFile
		);
		break;
	}
	case Maple2::Identifier::PS2F:
	{
		MakePackFile<Maple2::PackTraits::PS2F>(
			TargetFolder,
			HeaderFile,
			DataFile
		);
		break;
	}
	}

	return true;
}
