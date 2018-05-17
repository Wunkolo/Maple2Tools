#pragma once

#include <cstdint>

namespace Maple2
{

extern const std::uint8_t MS2F_Key_LUT[128][32];
extern const std::uint8_t MS2F_IV_LUT[128][16];
extern const std::uint8_t MS2F_XOR_Key[2048];

extern const std::uint8_t NS2F_Key_LUT[128][32];
extern const std::uint8_t NS2F_IV_LUT[128][16];

extern const std::uint8_t OS2F_Key_LUT[128][32];
extern const std::uint8_t OS2F_IV_LUT[128][16];

extern const std::uint8_t PS2F_Key_LUT[128][32];
extern const std::uint8_t PS2F_IV_LUT[128][16];

}
