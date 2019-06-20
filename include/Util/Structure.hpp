#pragma once
#include <cstdint>
#include <cstddef>

// Utility structures used for building structures at specified sizes and
// offsets

namespace Util
{

#pragma pack(push,1)

// Padding structure used to guarantee the size of a structure
// Ex:
//	union MyType
//	{
//	public:
//		(Some Fields)
//	private:
//		Padding<0x80> Pad;
//	};
template< std::size_t Size >
struct Padding
{
private:
	std::uint8_t PadBytes[Size];
	static_assert(
		sizeof(decltype(PadBytes)) == Size,
		"Padding size not being enforced"
	);
};

// Declares a data type at the specified offset
// Intended to be used within a union
// Ex:
//	union MyType
//	{
//		Field<0x80,std::uint32_t> Integer80;
//		Field<0x90,std::uint32_t> Integer90;
//	} Foo;
//	Foo.Integer80()++;
template< std::size_t FieldOffset, typename FieldType >
struct Field
{
private:
	// Forcecs Data to be at specified offset
	Padding<FieldOffset> Pad;
public:
	FieldType Data;
	using Type = FieldType;
	static constexpr std::size_t Offset = FieldOffset;
	FieldType& operator() ()
	{
		static_assert(
			offsetof(Field,Data) == FieldOffset, "Field offset not being enforced"
		);
		return Data;
	}
private:
};

#pragma pack(pop)

}
