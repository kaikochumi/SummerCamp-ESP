// config.cpp — plain text file persistence (one key=value per line).
#include "config.h"

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <string>

namespace {

bool read_bool(const std::string& v, bool d) {
    return v == "1" || v == "true" || v == "on";
}

uint32_t read_color(const std::string& v, uint32_t d) {
    if (v.empty()) return d;
    return static_cast<uint32_t>(std::strtoul(v.c_str(), nullptr, 0));
}

}  // namespace

void Config::load(const std::string& path) {
    std::ifstream in(path);
    if (!in) return;
    std::string line;
    while (std::getline(in, line)) {
        const size_t eq = line.find('=');
        if (eq == std::string::npos) continue;
        const std::string k = line.substr(0, eq);
        const std::string v = line.substr(eq + 1);

        if (k == "topmost") topmost = read_bool(v, topmost);
        else if (k == "esp_enabled") esp_enabled = read_bool(v, esp_enabled);
        else if (k == "esp_box") esp_box = read_bool(v, esp_box);
        else if (k == "box_style") box_style = static_cast<BoxStyle>(std::atoi(v.c_str()));
        else if (k == "esp_outline") esp_outline = read_bool(v, esp_outline);
        else if (k == "esp_fill") esp_fill = read_bool(v, esp_fill);
        else if (k == "esp_name") esp_name = read_bool(v, esp_name);
        else if (k == "esp_health") esp_health = read_bool(v, esp_health);
        else if (k == "esp_stamina") esp_stamina = read_bool(v, esp_stamina);
        else if (k == "esp_distance") esp_distance = read_bool(v, esp_distance);
        else if (k == "esp_snapline") esp_snapline = read_bool(v, esp_snapline);
        else if (k == "esp_head_dot") esp_head_dot = read_bool(v, esp_head_dot);
        else if (k == "esp_crosshair") esp_crosshair = read_bool(v, esp_crosshair);
        else if (k == "esp_max_distance") esp_max_distance = std::strtof(v.c_str(), nullptr);
        else if (k == "esp_min_box_height")
            esp_min_box_height = std::strtof(v.c_str(), nullptr);
        else if (k == "esp_box_thickness") esp_box_thickness = std::strtof(v.c_str(), nullptr);
        else if (k == "esp_box_width") esp_box_width = std::strtof(v.c_str(), nullptr);
        else if (k == "esp_text_scale") esp_text_scale = std::strtof(v.c_str(), nullptr);
        else if (k == "esp_text_size") esp_text_size = std::atoi(v.c_str());
        else if (k == "esp_crosshair_size") esp_crosshair_size = std::strtof(v.c_str(), nullptr);
        else if (k == "esp_box_color") esp_box_color = read_color(v, esp_box_color);
        else if (k == "esp_outline_color") esp_outline_color = read_color(v, esp_outline_color);
        else if (k == "esp_fill_color") esp_fill_color = read_color(v, esp_fill_color);
        else if (k == "esp_name_color") esp_name_color = read_color(v, esp_name_color);
        else if (k == "esp_snapline_color") esp_snapline_color = read_color(v, esp_snapline_color);
        else if (k == "esp_head_dot_color") esp_head_dot_color = read_color(v, esp_head_dot_color);
        else if (k == "esp_health_color") esp_health_color = read_color(v, esp_health_color);
        else if (k == "esp_stamina_color") esp_stamina_color = read_color(v, esp_stamina_color);
        else if (k == "esp_crosshair_color") esp_crosshair_color = read_color(v, esp_crosshair_color);
        else if (k == "esp_only_visible") esp_only_visible = read_bool(v, esp_only_visible);
        else if (k == "esp_skeleton") esp_skeleton = read_bool(v, esp_skeleton);
        else if (k == "esp_skeleton_thickness")
            esp_skeleton_thickness = std::strtof(v.c_str(), nullptr);
        else if (k == "esp_skeleton_color")
            esp_skeleton_color = read_color(v, esp_skeleton_color);
        else if (k == "lang") lang = std::atoi(v.c_str());
        else if (k == "github_user") github_user = v;
        else if (k == "repo_url") repo_url = v;
    }
}

void Config::save(const std::string& path) const {
    std::ofstream out(path, std::ios::trunc);
    if (!out) return;
    auto b = [](bool v) { return v ? "1" : "0"; };
    out << "topmost=" << b(topmost) << "\n";
    out << "esp_enabled=" << b(esp_enabled) << "\n";
    out << "esp_box=" << b(esp_box) << "\n";
    out << "box_style=" << static_cast<int>(box_style) << "\n";
    out << "esp_outline=" << b(esp_outline) << "\n";
    out << "esp_fill=" << b(esp_fill) << "\n";
    out << "esp_name=" << b(esp_name) << "\n";
    out << "esp_health=" << b(esp_health) << "\n";
    out << "esp_stamina=" << b(esp_stamina) << "\n";
    out << "esp_distance=" << b(esp_distance) << "\n";
    out << "esp_snapline=" << b(esp_snapline) << "\n";
    out << "esp_head_dot=" << b(esp_head_dot) << "\n";
    out << "esp_crosshair=" << b(esp_crosshair) << "\n";
    out << "esp_only_visible=" << b(esp_only_visible) << "\n";
    out << "esp_skeleton=" << b(esp_skeleton) << "\n";
    out << "esp_skeleton_thickness=" << esp_skeleton_thickness << "\n";
    out << "esp_max_distance=" << esp_max_distance << "\n";
    out << "esp_min_box_height=" << esp_min_box_height << "\n";
    out << "esp_box_thickness=" << esp_box_thickness << "\n";
    out << "esp_box_width=" << esp_box_width << "\n";
    out << "esp_text_scale=" << esp_text_scale << "\n";
    out << "esp_text_size=" << esp_text_size << "\n";
    out << "esp_crosshair_size=" << esp_crosshair_size << "\n";
    out << "esp_box_color=0x" << std::hex << esp_box_color << std::dec << "\n";
    out << "esp_outline_color=0x" << std::hex << esp_outline_color << std::dec << "\n";
    out << "esp_fill_color=0x" << std::hex << esp_fill_color << std::dec << "\n";
    out << "esp_name_color=0x" << std::hex << esp_name_color << std::dec << "\n";
    out << "esp_snapline_color=0x" << std::hex << esp_snapline_color << std::dec << "\n";
    out << "esp_head_dot_color=0x" << std::hex << esp_head_dot_color << std::dec << "\n";
    out << "esp_health_color=0x" << std::hex << esp_health_color << std::dec << "\n";
    out << "esp_stamina_color=0x" << std::hex << esp_stamina_color << std::dec << "\n";
    out << "esp_crosshair_color=0x" << std::hex << esp_crosshair_color << std::dec << "\n";
    out << "esp_skeleton_color=0x" << std::hex << esp_skeleton_color << std::dec << "\n";
    out << "lang=" << lang << "\n";
    out << "github_user=" << github_user << "\n";
    out << "repo_url=" << repo_url << "\n";
}
