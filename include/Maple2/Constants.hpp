#pragma once
#include <cstdint>

namespace Maple2
{

constexpr std::uint32_t MakeMagic(
	std::uint8_t Byte1, std::uint8_t Byte2,
	std::uint8_t Byte3, std::uint8_t Byte4
)
{
	return static_cast<std::uint32_t>(
		static_cast<std::uint8_t>(Byte4) << 24 |
		static_cast<std::uint8_t>(Byte3) << 16 |
		static_cast<std::uint8_t>(Byte2) << 8  |
		static_cast<std::uint8_t>(Byte1)
	);
}

enum class Magic : std::uint32_t
{
	MS2F = MakeMagic( 'M', 'S', '2', 'F' ),
	NS2F = MakeMagic( 'N', 'S', '2', 'F' ),
	OS2F = MakeMagic( 'O', 'S', '2', 'F' ),
	PS2F = MakeMagic( 'P', 'S', '2', 'F' )
};

}
