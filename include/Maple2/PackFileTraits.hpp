#pragma once

#include "Constants.hpp"
#include "Keys.hpp"
#include "Types.hpp"

namespace Maple2
{

template< Magic Version >
struct PackFileTraits
{
};

template<>
struct PackFileTraits<Magic::MS2F>
{
	static constexpr Magic Identifier = Magic::MS2F;
	using StreamType = PackStreamVer1;
	using FileHeaderType = PackFileHeaderVer1;
	static constexpr auto IV_LUT = MS2F_IV_LUT;
	static constexpr auto Key_LUT = MS2F_Key_LUT;
};

template<>
struct PackFileTraits<Magic::NS2F>
{
	static constexpr Magic Identifier = Magic::NS2F;
	using StreamType = PackStreamVer2;
	using FileHeaderType = PackFileHeaderVer2;
	static constexpr auto IV_LUT = NS2F_IV_LUT;
	static constexpr auto Key_LUT = NS2F_Key_LUT;
};

template<>
struct PackFileTraits<Magic::OS2F>
{
	static constexpr Magic Identifier = Magic::OS2F;
	using StreamType = PackStreamVer3;
	using FileHeaderType = PackFileHeaderVer3;
	static constexpr auto IV_LUT = OS2F_IV_LUT;
	static constexpr auto Key_LUT = OS2F_Key_LUT;
};

template<>
struct PackFileTraits<Magic::PS2F>
{
	static constexpr Magic Identifier = Magic::PS2F;
	using StreamType = PackStreamVer3;
	using FileHeaderType = PackFileHeaderVer3;
	static constexpr auto IV_LUT = PS2F_IV_LUT;
	static constexpr auto Key_LUT = PS2F_Key_LUT;
};

typedef PackFileTraits<Magic::MS2F> MS2FTraits;
typedef PackFileTraits<Magic::NS2F> NS2FTraits;
typedef PackFileTraits<Magic::OS2F> OS2FTraits;
typedef PackFileTraits<Magic::PS2F> PS2FTraits;

}
