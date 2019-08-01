#pragma once

#include "Constants.hpp"
#include "Keys.hpp"
#include "Types.hpp"

namespace Maple2
{

// Pack file/archive traits

template< Identifier Version >
struct PackFile
{
};

template<>
struct PackFile<Identifier::MS2F>
{
	static constexpr Identifier Magic = Identifier::MS2F;
	using StreamType = PackStreamVer1;
	using FileHeaderType = PackFileHeaderVer1;
	static constexpr auto& IV_LUT = MS2F_IV_LUT;
	static constexpr auto& Key_LUT = MS2F_Key_LUT;
};

template<>
struct PackFile<Identifier::NS2F>
{
	static constexpr Identifier Magic = Identifier::NS2F;
	using StreamType = PackStreamVer2;
	using FileHeaderType = PackFileHeaderVer2;
	static constexpr auto& IV_LUT = NS2F_IV_LUT;
	static constexpr auto& Key_LUT = NS2F_Key_LUT;
};

template<>
struct PackFile<Identifier::OS2F>
{
	static constexpr Identifier Magic = Identifier::OS2F;
	using StreamType = PackStreamVer3;
	using FileHeaderType = PackFileHeaderVer3;
	static constexpr auto& IV_LUT = OS2F_IV_LUT;
	static constexpr auto& Key_LUT = OS2F_Key_LUT;
};

template<>
struct PackFile<Identifier::PS2F>
{
	static constexpr Identifier Magic = Identifier::PS2F;
	using StreamType = PackStreamVer3;
	using FileHeaderType = PackFileHeaderVer3;
	static constexpr auto& IV_LUT = PS2F_IV_LUT;
	static constexpr auto& Key_LUT = PS2F_Key_LUT;
};


namespace PackTraits
{
typedef PackFile<Identifier::MS2F> MS2F;
typedef PackFile<Identifier::NS2F> NS2F;
typedef PackFile<Identifier::OS2F> OS2F;
typedef PackFile<Identifier::PS2F> PS2F;
}
}
