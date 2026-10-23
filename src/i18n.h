// i18n.h — menu language support: English (default) and Japanese.
#pragma once

#include "imgui.h"

namespace i18n {

enum Lang { kEn = 0, kJa = 1 };

// Language from the config. The overlay fills the font pointers right
// after building the atlas; g_font_ja stays null on systems without a
// Japanese-capable font, and the menu then keeps English.
inline int g_lang = kEn;
inline ImFont* g_font_en = nullptr;
inline ImFont* g_font_ja = nullptr;

// Face the menu should draw with under the current language.
inline ImFont* menu_font() {
    if (g_lang == kJa && g_font_ja) return g_font_ja;
    return g_font_en;
}

// Japanese variant of key, or key itself in English. ImGui ids after
// "##" are carried over untouched. Keys missing from the table pass
// through unchanged.
const char* tr(const char* key);

}  // namespace i18n
