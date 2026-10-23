// main.cpp — the overlay main loop.
#include <windows.h>

#include <cstdio>
#include <string>

#include "config.h"
#include "game.h"
#include "i18n.h"
#include "imgui.h"
#include "memory.h"
#include "menu.h"
#include "overlay.h"

namespace {

bool find_game_window(DWORD pid, RECT* out) {
    struct Ctx {
        DWORD pid;
        RECT rect;
        long best;   // area of the largest window
        bool found;
    } ctx{pid, {}, 0, false};

    EnumWindows(
        [](HWND hwnd, LPARAM lp) -> BOOL {
            Ctx& c = *reinterpret_cast<Ctx*>(lp);
            DWORD wpid = 0;
            GetWindowThreadProcessId(hwnd, &wpid);
            if (wpid != c.pid) return TRUE;
            if (!IsWindowVisible(hwnd)) return TRUE;
            RECT r{};
            if (!GetClientRect(hwnd, &r)) return TRUE;
            const long area = static_cast<long>(r.right) * r.bottom;
            if (r.right <= 0 || r.bottom <= 0) return TRUE;
            // the game opens several windows; the one we want is the largest
            if (area <= c.best) return TRUE;
            POINT pt{r.left, r.top};
            ClientToScreen(hwnd, &pt);
            c.rect = {pt.x, pt.y, pt.x + r.right, pt.y + r.bottom};
            c.best = area;
            c.found = true;
            return TRUE;
        },
        reinterpret_cast<LPARAM>(&ctx));
    if (ctx.found && out) *out = ctx.rect;
    return ctx.found;
}

// Keys with edge detection, so holding does not repeat the action.
struct Hotkey {
    bool prev = false;
    bool pressed(int vk) {
        const bool down = (GetAsyncKeyState(vk) & 0x8000) != 0;
        const bool edge = down && !prev;
        prev = down;
        return edge;
    }
};

}  // namespace

int main(int argc, char** argv) {
    setvbuf(stdout, nullptr, _IONBF, 0);
    std::string module_hint = "summercamp";
    std::string cfg_path = "esp.cfg";
    for (int i = 1; i < argc; ++i) {
        const std::string a = argv[i];
        if (a == "--module" && i + 1 < argc) module_hint = argv[++i];
        else if (a == "--cfg" && i + 1 < argc) cfg_path = argv[++i];
        else if (a == "--help") {
            printf("usage: summercamp-esp [--module NAME] [--cfg FILE]\n");
            return 0;
        }
    }

    Config cfg;
    cfg.load(cfg_path);
    cfg.menu_visible = true;
    i18n::g_lang = cfg.lang;

    mem::Process proc;
    game::Game g;
    g.set_auto_target(true);   // the ESP detects its own target

    printf("looking for the game...\n");
    const bool attached = g.attach(&proc, module_hint.c_str());
    if (attached) {
        printf("game: %s (pid %lu)\n", proc.path().c_str(), proc.pid());
    } else {
        printf("game not found; the menu opens anyway and reconnects once "
               "it starts.\n");
    }

    Overlay ov;
    RECT wr{};
    if (attached) {
        for (int tries = 0; tries < 100 && !find_game_window(proc.pid(), &wr); ++tries)
            Sleep(100);
    }
    if (wr.right > 0) {
        const int w = wr.right - wr.left;
        const int h = wr.bottom - wr.top;
        if (!ov.create(wr.left, wr.top, w, h, cfg.topmost)) {
            printf("falha ao criar o overlay\n");
            return 1;
        }
        g.set_viewport(w, h);
        printf("overlay: %dx%d at (%d,%d)\n", w, h, wr.left, wr.top);
        printf("overlay hwnd: %p\n", (void*)ov.hwnd());
    } else {
        printf("game window not found; covering the whole screen\n");
        const int w = GetSystemMetrics(SM_CXSCREEN);
        const int h = GetSystemMetrics(SM_CYSCREEN);
        if (!ov.create(0, 0, w, h, cfg.topmost)) {
            printf("failed to create the overlay\n");
            return 1;
        }
        g.set_viewport(w, h);
    }
    ov.set_click_through(true);

    Hotkey key_menu, key_esp, key_quit;
    float menu_rect[4] = {0, 0, 0, 0};

    while (ov.pump()) {
        if (key_menu.pressed(VK_INSERT)) cfg.menu_visible = !cfg.menu_visible;
        if (key_esp.pressed(VK_END)) cfg.esp_enabled = !cfg.esp_enabled;
        if (key_quit.pressed(VK_DELETE)) break;

        // If the game was not running, retry every now and then.
        if (!g.ready()) {
            if (g.attach(&proc, module_hint.c_str())) {
                printf("game: %s (pid %lu)\n", proc.path().c_str(), proc.pid());
                RECT r{};
                if (find_game_window(proc.pid(), &r) && r.right > 0) {
                    ov.move(r.left, r.top, r.right - r.left, r.bottom - r.top);
                    g.set_viewport(r.right - r.left, r.bottom - r.top);
                }
            }
        }

        g.refresh();

        ov.begin_frame();
        ui::draw_esp(cfg, g, ov.width(), ov.height());
        if (cfg.menu_visible) {
            // save as soon as something changes, not only at exit: a
            // crash must not take the config down with it
            if (ui::draw_menu(cfg, g, menu_rect)) cfg.save(cfg_path);
        }
        ov.end_frame();

        // The overlay covers the whole screen. It only accepts clicks when
        // the cursor is over the menu; otherwise the game gets the mouse
        // as normal.
        const ImVec2 mp = ImGui::GetIO().MousePos;
        const bool over_menu = cfg.menu_visible && menu_rect[2] > 0.f &&
                               mp.x >= menu_rect[0] && mp.x <= menu_rect[2] &&
                               mp.y >= menu_rect[1] && mp.y <= menu_rect[3];
        ov.set_click_through(!over_menu);
    }

    cfg.save(cfg_path);
    ov.destroy();
    return 0;
}
