# Maple2Tools [![GitHub license](https://img.shields.io/badge/license-MIT-blue.svg)](https://github.com/Wunkolo/Maple2Tools/blob/master/LICENSE)

Research and reverse engineering of Nexon's MapleStory 2. Check out the [XeNTaX](http://forum.xentax.com/viewtopic.php?f=10&t=18090) thread here.

## Dependencies

 * [Cmake 3.2.2+](https://www.cmake.org/download/)
 * [Crypto++ 7](https://github.com/weidai11/cryptopp)


# Building

Clone the repository:

`git clone https://github.com/Wunkolo/Maple2Tools`

## Windows

`TODO:` At the moment [getting Crypto++ to work with CMake](https://www.cryptopp.com/wiki/Cmake#CMake_Removal) is proving to be a feat that will require some manual tinkering to get this to build on windows. This is the only thing in the way of getting a concise Windows build as the rest of the code is portable and build-able on windows but a simple way to link Crypto++ requires some overhead. Feel free to contribute a pull request if you manage to get a build going.

## Linux

Typical [CMake out-of-source build procedure](http://preshing.com/20170511/how-to-build-a-cmake-based-project/#running-cmake-from-the-command-line):

```
cd Maple2Tools
mkdir build
cd build
cmake ..
make
```
All targets will then be generated in the `build` directory.

# Usage

## Expand
Expand will recursively dive into a target folder(such as your MapleStory2 game directory), and create symbolic links to all files it encounters. It will "soft-mirror" the target folder. For the ".m2h/.m2d" files it encounters it will instead create a folder of the same name full of its extracted contents(Ex: `Xml.m2h/Xml.m2d` will create an `Xml` folder with its unpacked contents).
This effectively flattens out the entire runtime virtual file system(internally named `CFileSystem`) of MapleStory 2.
This does the exact opposite of the `Pack` command.
The expander is very fast and multi-threaded and will create a new thread to extract with in parallel for every `.m2h/.m2d` pair it encounters.
It is recommended to run this on non-mechanical memory such as an SSD or RAM drive.

`./Expand MapleFiles Dump`

```
MapleStory2 Filesystem expander:
	"Flattens" a filesystem, expanding all m2h/m2d files it encounters
	into a folder of the same name
Build Date: Fri May 25 19:07:40 2018
	- wunkolo <wunkolo@gmail.com>
Usage: Expand (Source) (Dest)

File: MapleFiles/appdata/Data/Resource/Model/Path.m2h
Magic: 4632534d ( `MS2F` )
FATCompressedSize: 3a8 ( 936 )
FATEncodedSize: 4e0 ( 1248 )
FileListSize: a2a ( 2602 )
FileListCompressedSize: 1fc ( 508 )
FileListEncodedSize: 2a8 ( 680 )
TotalFiles: 4e ( 78 )
FATSize: ea0 ( 3744 )
Dump/MapleFiles/appdata/Data/Resource/Model/Path/bat_path01_1-mesh.nif
Dump/MapleFiles/appdata/Data/Resource/Model/Path/bat_path01_1.nif
Dump/MapleFiles/appdata/Data/Resource/Model/Path/bat_path01_2-mesh.nif
Dump/MapleFiles/appdata/Data/Resource/Model/Path/bat_path01_2.nif
Dump/MapleFiles/appdata/Data/Resource/Model/Path/bat_path02_1-mesh.nif
Dump/MapleFiles/appdata/Data/Resource/Model/Path/bat_path02_1.nif
Dump/MapleFiles/appdata/Data/Resource/Model/Path/bat_path02_2-mesh.nif
Dump/MapleFiles/appdata/Data/Resource/Model/Path/bat_path02_2.nif
Dump/MapleFiles/appdata/Data/Resource/Model/Path/coupleaction_dummy_bowdown_diff_loop.nif
Dump/MapleFiles/appdata/Data/Resource/Model/Path/coupleaction_dummy_bowdown_diff_ready.nif
...
```
```
tree -d Dump | head -n 50
Dump
└── MapleFiles
    ├── appdata
    │   ├── BlackCipher
    │   ├── Custom
    │   │   ├── Cube
    │   │   └── Equip
    │   ├── Data
    │   │   ├── lua
    │   │   │   └── Precompiled
    │   │   ├── Resource
    │   │   │   ├── asset-web-config
    │   │   │   ├── asset-web-metadata
    │   │   │   ├── Exported
    │   │   │   │   ├── flat
    │   │   │   │   │   ├── library
    │   │   │   │   │   │   ├── beastmodellibrary
    │   │   │   │   │   │   ├── gimodellibrary
    │   │   │   │   │   │   ├── maplestory2library
    │   │   │   │   │   │   ├── physxmodellibrary
    │   │   │   │   │   │   ├── standardmodellibrary
    │   │   │   │   │   │   ├── tool
    │   │   │   │   │   │   └── triggerslibrary
    │   │   │   │   │   └── presets
    │   │   │   │   │       ├── presets common
    │   │   │   │   │       ├── presets cube
    │   │   │   │   │       ├── presets cube_shadow
    │   │   │   │   │       ├── presets npc
    │   │   │   │   │       ├── presets object
    │   │   │   │   │       ├── presets object_shadow
    │   │   │   │   │       └── presets test
    │   │   │   │   └── xblock
    │   │   │   ├── gfx
    │   │   │   ├── Gfx
    │   │   │   ├── Image
    │   │   │   │   ├── action
    │   │   │   │   ├── adventurelevel
    │   │   │   │   ├── attendgift
    │   │   │   │   ├── bg
    │   │   │   │   ├── birthdaycard
    │   │   │   │   ├── character
    │   │   │   │   ├── characterability
    │   │   │   │   ├── effect
    │   │   │   │   ├── emblem
    │   │   │   │   │   ├── medal
    │   │   │   │   │   │   └── icon
    │   │   │   │   │   ├── npc
    │   │   │   │   │   │   └── icon
    │   │   │   │   │   ├── server
    │   │   │   │   │   └── symbol_shop
...
```
## Pack

`Pack` will dive into a directory and will recursively add each file into a `.m2h/.m2d` archive pair using the specified `PackStream` version.
Pretty much does the exact opposite of `Expand`.
```
./Pack MS2F Assets/MyFiles
MapleStory2 Filesystem packer:
	"Packs" a filesystem, creating an .m2h/.m2d pair
	of the original folder's name
Build Date: Tue May 29 23:30:07 2018
	- wunkolo <wunkolo@gmail.com>
Usage: Pack (M2SF/N2SF/O2SF/P2SF) (List of folders to pack)

Creating header file: ...Assets/MyFiles.m2h
Creating data file: ...Assets/MyFiles.m2d
1,blah.txt
2,Folder1/test.txt
3,.hidden

```
```
tree Assets
Assets/
├── MyFiles
│   ├── Folder1
│   │   └── test.txt
│   └── blah.txt
├── MyFiles.m2d
└── MyFiles.m2h
```
