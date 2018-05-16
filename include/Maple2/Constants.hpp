#pragma once


namespace Maple2
{

namespace {

constexpr std::uint32_t MakeMagic(
	char Byte1,	char Byte2,
	char Byte3,	char Byte4
)
{
	return static_cast<std::uint32_t>(
		static_cast<std::uint8_t>(Byte1) << 24 |
		static_cast<std::uint8_t>(Byte2) << 16 |
		static_cast<std::uint8_t>(Byte3) << 8  |
		static_cast<std::uint8_t>(Byte4)
	);
}

}
enum class Magic : std::uint32_t
{
	MS2F = ::MakeMagic( 'M', 'S', '2', 'F' ),
	NS2F = ::MakeMagic( 'N', 'S', '2', 'F' ),
	OS2F = ::MakeMagic( 'O', 'S', '2', 'F' ),
	PS2F = ::MakeMagic( 'P', 'S', '2', 'F' )
};

}
