#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <vector>

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
		"\tor the original folder's name\n"
		"Build Date: " __TIMESTAMP__ "\n"
		"\t- wunkolo <wunkolo@gmail.com>\n"
		"Usage: Pack (List of folders to pack)\n"
	);
	if( argc < 2 )
	{
		std::puts("No argument given");
		return EXIT_FAILURE;
	}

	for( std::size_t i = 1; i < static_cast<std::size_t>(argc); ++i )
	{
		PackFolder( argv[i], Maple2::Identifier::MS2F );
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

	for( 
		const auto& CurFile
		: fs::recursive_directory_iterator(
			TargetFolder,
			fs::directory_options::follow_directory_symlink
		)
	)
	{
		StreamHeader.TotalFiles++;
		const auto& CurPath = CurFile.path();
		const fs::path RelativePath = fs::path(
			CurPath.wstring().substr(
				TargetFolder.wstring().size() + 1
			)
		);

		std::size_t EncodedSize;
		std::size_t CompressedSize;
		EncodedSize = CompressedSize = 0;

		typename PackTraits::FileHeaderType CurFATEntry = {};
		CurFATEntry.FileIndex = StreamHeader.TotalFiles;
		CurFATEntry.Compression = Maple2::CompressionType::Deflate;
		CurFATEntry.CompressedSize = CompressedSize;
		CurFATEntry.EncodedSize = EncodedSize;
		CurFATEntry.Size = fs::file_size( CurPath );

		std::printf(
			"%zu,%ls\n",
			static_cast<std::size_t>(StreamHeader.TotalFiles),
			RelativePath.wstring().c_str()
		);
		FileTable.push_back(CurFATEntry);
	}

	StreamHeader.FATCompressedSize;      // DEFLATE length
	StreamHeader.FATEncodedSize;         // Base64 length
	StreamHeader.FATSize;                // Uncompressed size
	StreamHeader.FileListCompressedSize; // DEFLATE length
	StreamHeader.FileListEncodedSize;    // Base64 length
	StreamHeader.FileListSize;           // Uncompressed size

	// Write header
	Util::Write(HeaderFile, PackTraits::Magic);
	Util::Write(HeaderFile, StreamHeader);

	// Write file allocation table

	// Write filelist


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
