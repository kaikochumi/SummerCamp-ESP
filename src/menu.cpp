// menu.cpp — ESP and menu.
#include "menu.h"

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <shellapi.h>
#include <string>
#include <vector>

#include "bones.h"
#include "fonts.h"
#include "i18n.h"
#include "imgui.h"
#include "version.h"

namespace ui {
namespace {

const ImVec4 kPink(0.98f, 0.45f, 0.68f, 1.f);
const ImVec4 kAmber(0.92f, 0.68f, 0.30f, 1.f);
const ImVec4 kGreen(0.40f, 0.88f, 0.52f, 1.f);

ImVec4 abgr_to_rgba(uint32_t c) {
    return ImVec4((c & 0xFF) / 255.f, ((c >> 8) & 0xFF) / 255.f,
                  ((c >> 16) & 0xFF) / 255.f, ((c >> 24) & 0xFF) / 255.f);
}

uint32_t pack_abgr(const ImVec4& c) {
    return (static_cast<uint32_t>(c.w * 255.f) << 24) |
           (static_cast<uint32_t>(c.x * 255.f)) |
           (static_cast<uint32_t>(c.y * 255.f) << 8) |
           (static_cast<uint32_t>(c.z * 255.f) << 16);
}

// The inline ColorEdit4 has 4 numeric fields and takes ~200px, which
// overflows the window. Here it is a color button that opens the picker
// in a popup.
void color_edit(const char* label, const char* id, uint32_t* v) {
    ImVec4 c = abgr_to_rgba(*v);
    const std::string popup = std::string("cp_") + id;

    if (ImGui::ColorButton(id, c, ImGuiColorEditFlags_AlphaBar, ImVec2(24.f, 18.f)))
        ImGui::OpenPopup(popup.c_str());
    ImGui::SameLine();
    ImGui::TextUnformatted(label);
    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        ImGui::OpenPopup(popup.c_str());

    if (ImGui::BeginPopup(popup.c_str())) {
        float arr[4] = {c.x, c.y, c.z, c.w};
        ImGui::ColorPicker4("##cp", arr, ImGuiColorEditFlags_AlphaBar |
                                         ImGuiColorEditFlags_AlphaPreviewHalf);
        c = ImVec4(arr[0], arr[1], arr[2], arr[3]);
        ImGui::EndPopup();
        *v = pack_abgr(c);
    }
}

// Slider with the label on the left. A full-width slider pushes the
// label out of the window and the text disappears.
void slider_row(const char* id, const char* label, float* v, float min, float max,
                const char* fmt) {
    if (ImGui::BeginTable(id, 2, ImGuiTableFlags_SizingStretchProp)) {
        ImGui::TableSetupColumn("##l", ImGuiTableColumnFlags_WidthFixed, 118.f);
        ImGui::TableSetupColumn("##v", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(label);
        ImGui::TableNextColumn();
        ImGui::SliderFloat("##s", v, min, max, fmt);
        ImGui::EndTable();
    }
}

// Short name for the enforced class, so the menu does not leak internal
// names onto the screen.
std::string target_label(const std::string& cls) {
    if (cls == "SummerCamp.SCCounselorCharacter") return i18n::tr("Counselors");
    if (cls == "SummerCamp.SCKillerCharacter") return i18n::tr("Jason");
    if (cls.empty()) return "-";
    return cls;
}

// ESP text with a 1px shadow: the scenery goes from light to dark, and
// without the shadow the label vanishes over bright scenes.
void tag_text(ImDrawList* dl, ImFont* font, float ts, const ImVec2& pos,
              const char* s, ImU32 col) {
    dl->AddText(font, ts, ImVec2(pos.x + 1.f, pos.y + 1.f), 0xB4000000, s);
    dl->AddText(font, ts, pos, col, s);
}

void box_shape(ImDrawList* dl, float x0, float y0, float x1, float y1,
               ImU32 col, float th, BoxStyle style) {
    switch (style) {
        case BoxStyle::Cantos: {
            const float len = (x1 - x0) * 0.3f;
            const float vlen = (y1 - y0) * 0.3f;
            dl->AddLine(ImVec2(x0, y0), ImVec2(x0 + len, y0), col, th);
            dl->AddLine(ImVec2(x0, y0), ImVec2(x0, y0 + vlen), col, th);
            dl->AddLine(ImVec2(x1, y0), ImVec2(x1 - len, y0), col, th);
            dl->AddLine(ImVec2(x1, y0), ImVec2(x1, y0 + vlen), col, th);
            dl->AddLine(ImVec2(x0, y1), ImVec2(x0 + len, y1), col, th);
            dl->AddLine(ImVec2(x0, y1), ImVec2(x0, y1 - vlen), col, th);
            dl->AddLine(ImVec2(x1, y1), ImVec2(x1 - len, y1), col, th);
            dl->AddLine(ImVec2(x1, y1), ImVec2(x1, y1 - vlen), col, th);
            break;
        }
        case BoxStyle::Arredondada:
            dl->AddRect(ImVec2(x0, y0), ImVec2(x1, y1), col, 6.f, 0, th);
            break;
        case BoxStyle::Solida:
        default:
            dl->AddRect(ImVec2(x0, y0), ImVec2(x1, y1), col, 0.f, 0, th);
            break;
    }
}

uint32_t health_color(float ratio) {
    if (ratio > 0.5f) return 0xFF00CC00;
    if (ratio > 0.25f) return 0xFFDDCC00;
    return 0xFFDD3333;
}

ImFont* text_font(const Config& cfg) {
    ImGuiIO& io = ImGui::GetIO();
    // 0 = menu, 1..3 = ESP sizes
    const int idx = std::clamp(cfg.esp_text_size + 1, 0,
                               static_cast<int>(io.Fonts->Fonts.size()) - 1);
    return io.Fonts->Fonts[idx];
}

// The sizes live in a table, not on ImFont, because LegacySize depends
// on how the font was created.
float text_size(const Config& cfg) {
    constexpr float kSizes[3] = {fonts::kEspSmall, fonts::kEspMedium,
                                 fonts::kEspLarge};
    return kSizes[std::clamp(cfg.esp_text_size, 0, 2)] * cfg.esp_text_scale;
}

void section(const char* title) {
    ImGui::Spacing();
    ImGui::TextDisabled("%s", title);
}

}  // namespace

void draw_esp(const Config& cfg, const game::Game& g, int view_w, int view_h) {
    if (!cfg.esp_enabled) return;

    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    const ImVec2 center(view_w * 0.5f, view_h * 0.5f);
    ImFont* font = text_font(cfg);
    const float ts = text_size(cfg);
    static std::vector<game::Bone> bone_pts;

    for (const auto& e : g.targets()) {
        const float meters = e.distance / 100.f;
        if (meters < 0.5f || meters > cfg.esp_max_distance) continue;
        if (!e.in_front || !e.on_screen) continue;
        if (cfg.esp_only_visible && !e.visible) continue;

        // distant targets shrink to a few-pixel sliver; the height floor
        // keeps the marker visible at any distance
        const float h = std::max(e.screen_h, cfg.esp_min_box_height);
        if (h < 2.f || h > view_h * 4.f) continue;

        const float w = h * cfg.esp_box_width;
        const float x0 = e.screen_x - w * 0.5f;
        const float x1 = e.screen_x + w * 0.5f;
        const float y0 = e.screen_y - h;   // head
        const float y1 = e.screen_y;       // feet

        // skeleton: connects bones in the character's bone order
        if (cfg.esp_skeleton) {
            const int nb = g.skeleton(e, bone_pts);
            if (nb >= bones::kCount) {
                for (int i = 0; i < bones::kEdgeCount; ++i) {
                    const game::Bone& a = bone_pts[bones::kEdges[i].a];
                    const game::Bone& b = bone_pts[bones::kEdges[i].b];
                    if (a.parent == -2 || b.parent == -2) continue;  // off screen
                    dl->AddLine(ImVec2(a.sx, a.sy), ImVec2(b.sx, b.sy),
                                cfg.esp_skeleton_color, cfg.esp_skeleton_thickness);
                }
            }
        }

        if (cfg.esp_box) {
            if (cfg.esp_fill) {
                dl->AddRectFilled(ImVec2(x0, y0), ImVec2(x1, y1), cfg.esp_fill_color);
            }
            if (cfg.esp_outline) {
                const float o = cfg.esp_box_thickness + 1.f;
                box_shape(dl, x0 - o, y0 - o, x1 + o, y1 + o, cfg.esp_outline_color,
                          cfg.esp_box_thickness + 1.f, cfg.box_style);
            }
            box_shape(dl, x0, y0, x1, y1, cfg.esp_box_color, cfg.esp_box_thickness,
                      cfg.box_style);
        }

        // health bar, stuck to the left side of the box
        if (cfg.esp_health && e.max_health > 0.f) {
            const float ratio = std::clamp(e.health / e.max_health, 0.f, 1.f);
            const float bw = 3.f;
            const float bx = x0 - bw - 1.f;
            dl->AddRectFilled(ImVec2(bx, y0), ImVec2(bx + bw, y1), 0x70000000);
            const float fh = (y1 - y0) * ratio;
            dl->AddRectFilled(ImVec2(bx, y1 - fh), ImVec2(bx + bw, y1),
                              health_color(ratio));
        }

        // stamina bar, on the right
        if (cfg.esp_stamina && e.max_stamina > 0.f) {
            const float ratio = std::clamp(e.stamina / e.max_stamina, 0.f, 1.f);
            const float bw = 2.f;
            const float bx = x1 + 2.f;
            dl->AddRectFilled(ImVec2(bx, y0), ImVec2(bx + bw, y1), 0x70000000);
            const float fh = (y1 - y0) * ratio;
            dl->AddRectFilled(ImVec2(bx, y1 - fh), ImVec2(bx + bw, y1),
                              cfg.esp_stamina_color);
        }

        if (cfg.esp_snapline) {
            dl->AddLine(center, ImVec2(e.screen_x, y1), cfg.esp_snapline_color, 1.f);
        }

        if (cfg.esp_head_dot) {
            dl->AddCircleFilled(ImVec2(e.screen_x, y0), 2.5f, cfg.esp_head_dot_color);
        }

        // label in the style of the big cheats: name centered above the
        // box and distance centered below it, no background — text with a
        // 1px shadow only. Distance in muted amber.
        char distbuf[16];
        const bool has_name = cfg.esp_name && !e.name.empty();
        const bool has_dist = cfg.esp_distance;
        if (has_dist) snprintf(distbuf, sizeof(distbuf), "%.0fm", meters);
        if (has_name || has_dist) {
            const ImVec2 sn = has_name
                ? font->CalcTextSizeA(ts, FLT_MAX, 0.f, e.name.c_str())
                : ImVec2(0.f, ts);
            const float sd = has_dist
                ? font->CalcTextSizeA(ts, FLT_MAX, 0.f, distbuf).x
                : 0.f;
            // name: aligned and fitted, 1px above the box
            float ty = y0 - sn.y - 1.f;
            if (ty < 2.f) ty = 2.f;    // target at the top of the screen
            if (has_name)
                tag_text(dl, font, ts, ImVec2(e.screen_x - sn.x * 0.5f, ty),
                         e.name.c_str(), cfg.esp_name_color);
            if (has_dist)
                tag_text(dl, font, ts,
                         ImVec2(e.screen_x - sd * 0.5f, y1 + 2.f), distbuf,
                         IM_COL32(226, 186, 96, 255));
        }
    }

    if (cfg.esp_crosshair) {
        const float s = cfg.esp_crosshair_size;
        dl->AddLine(ImVec2(center.x - s, center.y), ImVec2(center.x + s, center.y),
                    cfg.esp_crosshair_color, 1.f);
        dl->AddLine(ImVec2(center.x, center.y - s), ImVec2(center.x, center.y + s),
                    cfg.esp_crosshair_color, 1.f);
    }
}

bool draw_menu(Config& cfg, game::Game& g, float out_rect[4]) {
    bool changed = false;
    ImGui::GetIO().FontDefault = i18n::menu_font();

    // Combo entries change with the language.
    const char* box_styles[] = {
        i18n::tr("Solid"), i18n::tr("Corners"), i18n::tr("Rounded")};
    const char* text_sizes[] = {
        i18n::tr("Small"), i18n::tr("Medium"), i18n::tr("Large")};

    // Fixed size: no AlwaysAutoResize. With auto-resize plus an item of
    // width "-1" the window feeds back on itself and inflates to the
    // whole screen.
    const ImVec2 disp = ImGui::GetIO().DisplaySize;
    ImGui::SetNextWindowSize(ImVec2(300.f, 430.f), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(disp.x - 320.f, 30.f), ImGuiCond_Always);
    if (!ImGui::Begin("SummerCamp ESP", &cfg.menu_visible, ImGuiWindowFlags_NoResize)) {
        ImGui::End();
        return false;
    }

    // header: user, version and a help button
    ImGui::TextColored(kPink, "github.com/%s", cfg.github_user.c_str());
    ImGui::SameLine(ImGui::GetContentRegionAvail().x - 74.f);
    ImGui::TextDisabled("v" SCESP_VERSION);
    ImGui::SameLine();
    if (ImGui::SmallButton("?")) ImGui::OpenPopup("about");

    // status: this is why the ESP sometimes has nothing to draw
    if (!g.ready()) {
        ImGui::TextColored(kAmber, "%s", i18n::tr("Game not found"));
    } else if (!g.in_match()) {
        ImGui::TextColored(kAmber, "%s",
                           i18n::tr("No characters: join a match"));
    } else {
        ImGui::TextColored(kGreen, "%s", i18n::tr("In match"));
    }

    if (ImGui::BeginPopup("about")) {
        ImGui::Text("SummerCamp ESP v" SCESP_VERSION);
        ImGui::TextUnformatted(
            i18n::tr("Passive reading. Nothing is written to the game."));
        ImGui::Separator();
        ImGui::TextUnformatted(cfg.repo_url.c_str());
        if (ImGui::Button(i18n::tr("Open in browser"))) {
            ShellExecuteA(nullptr, "open", cfg.repo_url.c_str(), nullptr, nullptr,
                          SW_SHOWNORMAL);
        }
        ImGui::EndPopup();
    }

    if (ImGui::BeginTabBar("tabs")) {
        // ------------------------------------------------------------- ESP
        if (ImGui::BeginTabItem("ESP")) {
            ImGui::Checkbox(i18n::tr("Enable ESP"), &cfg.esp_enabled);
            ImGui::TextDisabled(i18n::tr("Target: %s"),
                                target_label(g.target_class()).c_str());

            section(i18n::tr("BOX"));
            ImGui::Checkbox(i18n::tr("Draw##box"), &cfg.esp_box);
            ImGui::SameLine();
            ImGui::Checkbox(i18n::tr("Black outline##out"), &cfg.esp_outline);
            ImGui::SameLine();
            ImGui::Checkbox(i18n::tr("Background##fill"), &cfg.esp_fill);
            ImGui::SetNextItemWidth(130.f);
            int style = static_cast<int>(cfg.box_style);
            if (ImGui::Combo(i18n::tr("Style##style"), &style, box_styles, 3)) {
                cfg.box_style = static_cast<BoxStyle>(style);
                changed = true;
            }

            section(i18n::tr("SKELETON"));
            ImGui::Checkbox(i18n::tr("Draw skeleton##skel"), &cfg.esp_skeleton);
            slider_row("##skth", i18n::tr("Thickness"),
                       &cfg.esp_skeleton_thickness, 0.5f, 5.f, "%.1f");

            section(i18n::tr("TEXT"));
            ImGui::Checkbox(i18n::tr("Player name##name"), &cfg.esp_name);
            ImGui::SameLine();
            ImGui::Checkbox(i18n::tr("Health##hp"), &cfg.esp_health);
            ImGui::Checkbox(i18n::tr("Stamina##st"), &cfg.esp_stamina);
            ImGui::SameLine();
            ImGui::Checkbox(i18n::tr("Distance##dist"), &cfg.esp_distance);
            ImGui::SetNextItemWidth(130.f);
            int tsz = std::clamp(cfg.esp_text_size, 0, 2);
            if (ImGui::Combo(i18n::tr("Size##tsz"), &tsz, text_sizes, 3)) {
                cfg.esp_text_size = tsz;
                changed = true;
            }

            section(i18n::tr("MISC"));
            ImGui::Checkbox(i18n::tr("Snapline##snap"), &cfg.esp_snapline);
            ImGui::SameLine();
            ImGui::Checkbox(i18n::tr("Head dot##head"), &cfg.esp_head_dot);
            ImGui::Checkbox(i18n::tr("Center crosshair##cross"),
                            &cfg.esp_crosshair);
            ImGui::SameLine();
            ImGui::Checkbox(i18n::tr("Visible only##vis"),
                            &cfg.esp_only_visible);
            ImGui::EndTabItem();
        }

        // ---------------------------------------------------------- Targets
        if (ImGui::BeginTabItem(i18n::tr("Targets"))) {
            const auto& list = g.targets();
            ImGui::Text(i18n::tr("%d targets  |  %d visible"), (int)list.size(),
                        g.visible_count());
            ImGui::Separator();
            if (ImGui::BeginTable("alvos", 4,
                                  ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                                      ImGuiTableFlags_SizingStretchProp)) {
                ImGui::TableSetupColumn(i18n::tr("Name"));
                ImGui::TableSetupColumn(i18n::tr("Distance"));
                ImGui::TableSetupColumn(i18n::tr("Health"));
                ImGui::TableSetupColumn(i18n::tr("Visible"));
                ImGui::TableHeadersRow();
                for (const auto& e : list) {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted(e.name.empty() ? "?" : e.name.c_str());
                    ImGui::TableNextColumn();
                    ImGui::Text("%.0f m", e.distance / 100.f);
                    ImGui::TableNextColumn();
                    if (e.max_health > 0.f) {
                        ImGui::Text("%.0f", e.health);
                    } else {
                        ImGui::TextUnformatted("-");
                    }
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted(i18n::tr(e.visible ? "yes" : "no"));
                }
                ImGui::EndTable();
            }
            ImGui::EndTabItem();
        }

        // ------------------------------------------------------------ Visual
        if (ImGui::BeginTabItem(i18n::tr("Visual"))) {
            slider_row("##maxd", i18n::tr("Max distance"), &cfg.esp_max_distance,
                       10.f, 2000.f, "%.0f m");
            slider_row("##minh", i18n::tr("Min height"),
                       &cfg.esp_min_box_height, 0.f, 60.f, "%.0f px");
            slider_row("##thick", i18n::tr("Thickness"),
                       &cfg.esp_box_thickness, 0.5f, 5.f, "%.1f");
            slider_row("##width", i18n::tr("Box width"), &cfg.esp_box_width,
                       0.2f, 1.2f, "%.2f");
            slider_row("##tscale", i18n::tr("Text scale"), &cfg.esp_text_scale,
                       0.5f, 2.f, "%.2f");
            slider_row("##cross", i18n::tr("Crosshair size"),
                       &cfg.esp_crosshair_size, 2.f, 30.f, "%.0f");

            section(i18n::tr("COLORS"));
            if (ImGui::SmallButton(i18n::tr("Open color picker"))) {
                ImGui::OpenPopup("colors_popup");
            }
            if (ImGui::BeginPopup("colors_popup")) {
                if (ImGui::BeginTable("cores", 2, ImGuiTableFlags_SizingStretchSame)) {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    color_edit(i18n::tr("Box"), "cc", &cfg.esp_box_color);
                    ImGui::TableNextColumn();
                    color_edit(i18n::tr("Outline"), "oc", &cfg.esp_outline_color);
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    color_edit(i18n::tr("Fill"), "fc", &cfg.esp_fill_color);
                    ImGui::TableNextColumn();
                    color_edit(i18n::tr("Health"), "hc", &cfg.esp_health_color);
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    color_edit(i18n::tr("Skeleton"), "skc",
                               &cfg.esp_skeleton_color);
                    ImGui::TableNextColumn();
                    color_edit(i18n::tr("Name"), "nc", &cfg.esp_name_color);
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    color_edit(i18n::tr("Stamina"), "sc",
                               &cfg.esp_stamina_color);
                    ImGui::TableNextColumn();
                    color_edit(i18n::tr("Line"), "slc",
                               &cfg.esp_snapline_color);
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    color_edit(i18n::tr("Head"), "hdc",
                               &cfg.esp_head_dot_color);
                    ImGui::TableNextColumn();
                    color_edit(i18n::tr("Crosshair"), "xc",
                               &cfg.esp_crosshair_color);
                    ImGui::EndTable();
                }
                ImGui::EndPopup();
            }

            section(i18n::tr("OVERLAY"));
            ImGui::Checkbox(i18n::tr("Keep above the game"), &cfg.topmost);
            ImGui::EndTabItem();
        }

        // ---------------------------------------------------------- Settings
        if (ImGui::BeginTabItem(i18n::tr("Settings"))) {
            ImGui::TextDisabled(i18n::tr("HOTKEYS"));
            ImGui::TextUnformatted(i18n::tr("INSERT  toggles the menu"));
            ImGui::TextUnformatted(i18n::tr("END     toggles the ESP"));
            ImGui::TextUnformatted(i18n::tr("DELETE  quits"));
            ImGui::Spacing();
            ImGui::TextDisabled(i18n::tr("ABOUT"));
            ImGui::TextUnformatted(
                i18n::tr("Passive read: the game is never modified."));
            ImGui::TextUnformatted(
                i18n::tr("Targets are detected automatically by your character."));
            ImGui::TextUnformatted(cfg.repo_url.c_str());
            if (ImGui::Button(i18n::tr("Open in browser"))) {
                ShellExecuteA(nullptr, "open", cfg.repo_url.c_str(), nullptr,
                              nullptr, SW_SHOWNORMAL);
            }
            ImGui::Spacing();
            ImGui::SetNextItemWidth(130.f);
            int lg = cfg.lang;
            const char* langs[] = {"English", "日本語", "Português"};
            if (ImGui::Combo("##lang", &lg, langs, 3)) {
                cfg.lang = lg;
                i18n::g_lang = lg;
                changed = true;
            }
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    if (out_rect) {
        const ImVec2 p = ImGui::GetWindowPos();
        const ImVec2 s = ImGui::GetWindowSize();
        out_rect[0] = p.x;
        out_rect[1] = p.y;
        out_rect[2] = p.x + s.x;
        out_rect[3] = p.y + s.y;
    }

    ImGui::End();
    return changed;
}

}  // namespace ui
