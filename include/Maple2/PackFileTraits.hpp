#pragma once

#include "Constants.hpp"
#include "Keys.hpp"
#include "Types.hpp"

namespace Maple2
{

template< Identifier Version >
struct PackFileTraits
{
};

template<>
struct PackFileTraits<Identifier::MS2F>
{
	static constexpr Identifier Identifier = Identifier::MS2F;
	using StreamType = PackStreamVer1;
	using FileHeaderType = PackFileHeaderVer1;
	static constexpr auto IV_LUT = MS2F_IV_LUT;
	static constexpr auto Key_LUT = MS2F_Key_LUT;
};

template<>
struct PackFileTraits<Identifier::NS2F>
{
	static constexpr Identifier Identifier = Identifier::NS2F;
	using StreamType = PackStreamVer2;
	using FileHeaderType = PackFileHeaderVer2;
	static constexpr auto IV_LUT = NS2F_IV_LUT;
	static constexpr auto Key_LUT = NS2F_Key_LUT;
};

template<>
struct PackFileTraits<Identifier::OS2F>
{
	static constexpr Identifier Identifier = Identifier::OS2F;
	using StreamType = PackStreamVer3;
	using FileHeaderType = PackFileHeaderVer3;
	static constexpr auto IV_LUT = OS2F_IV_LUT;
	static constexpr auto Key_LUT = OS2F_Key_LUT;
};

template<>
struct PackFileTraits<Identifier::PS2F>
{
	static constexpr Identifier Identifier = Identifier::PS2F;
	using StreamType = PackStreamVer3;
	using FileHeaderType = PackFileHeaderVer3;
	static constexpr auto IV_LUT = PS2F_IV_LUT;
	static constexpr auto Key_LUT = PS2F_Key_LUT;
};

typedef PackFileTraits<Identifier::MS2F> MS2FTraits;
typedef PackFileTraits<Identifier::NS2F> NS2FTraits;
typedef PackFileTraits<Identifier::OS2F> OS2FTraits;
typedef PackFileTraits<Identifier::PS2F> PS2FTraits;

}
