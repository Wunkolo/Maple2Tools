#pragma once

#include <cstddef>
#include <istream>
#include <ostream>

namespace Util
{

template< typename T >
inline T Read(std::istream& Stream)
{
	T Temp;
	Stream.read(
		reinterpret_cast<char*>(&Temp),
		sizeof(T)
	);
	return Temp;
}

inline void Read(std::istream& Stream, void* Data, std::size_t Size)
{
	Stream.read(
		reinterpret_cast<char*>(Data),
		Size
	);
}

template< typename T >
inline void Write(std::ostream& Stream, const T& Value)
{
	Stream.write(
		reinterpret_cast<char*>(Value),
		sizeof(T)
	);
}

inline void Write(std::ostream& Stream, void* Data, std::size_t Size)
{
	Stream.write(
		reinterpret_cast<char*>(Data),
		Size
	);
}

}
