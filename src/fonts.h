// fonts.h — ImGui font sizes.
//
// Atlas index 0 is the menu font. Indices 1, 2 and 3 are the sizes the
// ESP uses for target text.
//
// The sizes live here (not in ImFont::LegacySize) because LegacySize
// depends on how the font was created.
#pragma once

namespace fonts {

constexpr float kUi = 14.f;       // menu
constexpr float kEspSmall = 13.f; // ESP: small name/distance
constexpr float kEspMedium = 16.f;
constexpr float kEspLarge = 19.f;

}  // namespace fonts
