// config.h — overlay, menu and ESP settings.
#pragma once

#include <cstdint>
#include <string>

#include "i18n.h"

// How the target box is drawn.
enum class BoxStyle {
    Solida = 0,
    Cantos = 1,
    Arredondada = 2,
};

struct Config {
    // overlay
    bool topmost = true;

    // --- ESP: what is drawn -------------------------------------------------
    bool esp_enabled = true;
    bool esp_box = true;
    BoxStyle box_style = BoxStyle::Solida;
    bool esp_outline = false;
    bool esp_fill = false;
    bool esp_name = true;
    bool esp_health = true;
    bool esp_stamina = false;
    bool esp_distance = true;
    bool esp_snapline = false;
    bool esp_head_dot = false;      // dot above the head
    bool esp_crosshair = false;
    bool esp_only_visible = false;  // only characters being rendered on screen
    bool esp_skeleton = false;      // draws the character skeleton

    // --- ESP: sizes ---------------------------------------------------------
    // esp_max_distance is in METERS. The game returns distance in UE units
    // (cm); the conversion happens at filter time.
    float esp_max_distance = 300.f;
    float esp_min_box_height = 12.f;  // px: keeps distant targets visible
    float esp_box_thickness = 1.5f;
    float esp_box_width = 0.45f;   // width relative to height
    float esp_skeleton_thickness = 1.5f;
    float esp_text_scale = 1.0f;
    int esp_text_size = 0;         // 0 small, 1 medium, 2 large
    float esp_crosshair_size = 6.f;

    // --- colors (ABGR, as ImGui expects) ------------------------------------
    // defaults in the style of the big cheats: thin orange box,
    // green health, neutral elsewhere
    uint32_t esp_box_color = 0xFF2DA0FF;      // orange
    uint32_t esp_outline_color = 0xFF000000;
    uint32_t esp_fill_color = 0x2500FF00;
    uint32_t esp_name_color = 0xFFFFFFFF;
    uint32_t esp_snapline_color = 0xFF00FF00;
    uint32_t esp_head_dot_color = 0xFFFF5555;
    uint32_t esp_health_color = 0xFF00FF00;
    uint32_t esp_stamina_color = 0xFF3399FF;
    uint32_t esp_crosshair_color = 0xFFFFFFFF;
    uint32_t esp_skeleton_color = 0xFFCC00FF;

    // --- menu ---------------------------------------------------------------
    bool menu_visible = true;
    int lang = i18n::kEn;  // menu language: 0 English, 1 Japanese, 2 Portuguese
    std::string github_user = "kaikochumi";
    std::string repo_url = "https://github.com/kaikochumi/summercamp-esp";

    void load(const std::string& path);
    void save(const std::string& path) const;
};
