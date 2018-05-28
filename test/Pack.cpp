#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

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

	return EXIT_SUCCESS;
}
