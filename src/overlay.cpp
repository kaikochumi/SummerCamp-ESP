// overlay.cpp — transparent window: DWM for per-pixel alpha, D3D11 to
// render, ImGui on top.
#include "overlay.h"

#include <dwmapi.h>

#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"

#include "fonts.h"
#include "i18n.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

namespace {

constexpr DWORD kExTransparent = 0x00000020;  // WS_EX_TRANSPARENT
constexpr DWORD kExNoActivate = 0x08000000;   // WS_EX_NOACTIVATE

// How the window becomes transparent. DXGI draws with per-pixel alpha, but
// Windows only applies that alpha if the layered window is set up the
// right way. There are several recipes and not all of them work; the
// recipe in use is the one this machine accepts.
enum class AlphaRecipe {
    kDwmLayered,      // layered + DwmExtendFrameIntoClientArea
    kDwmAlphaAttr,    // layered + SetLayeredWindowAttributes + DWM
    kDwmPlain,        // no layered + DwmExtendFrameIntoClientArea
};

AlphaRecipe recipe_from_env() {
    const char* v = getenv("ESP_ALPHA");
    if (!v) return AlphaRecipe::kDwmAlphaAttr;
    const std::string s = v;
    if (s == "0") return AlphaRecipe::kDwmLayered;
    if (s == "2") return AlphaRecipe::kDwmPlain;
    return AlphaRecipe::kDwmAlphaAttr;
}

// Pink theme: dark base with a plum tint and pink accents.
void apply_pink_theme() {
    ImGuiStyle& s = ImGui::GetStyle();
    const ImVec4 pink(0.98f, 0.45f, 0.68f, 1.0f);
    const ImVec4 pink_mid(0.66f, 0.27f, 0.46f, 1.0f);
    const ImVec4 pink_dark(0.30f, 0.13f, 0.24f, 1.0f);
    const ImVec4 bg(0.10f, 0.06f, 0.10f, 0.95f);
    const ImVec4 bg_child(0.15f, 0.09f, 0.14f, 1.0f);
    const ImVec4 frame(0.21f, 0.12f, 0.19f, 1.0f);
    const ImVec4 frame_hover(0.31f, 0.17f, 0.27f, 1.0f);
    const ImVec4 frame_active(0.44f, 0.21f, 0.34f, 1.0f);
    const ImVec4 text(1.0f, 0.93f, 0.96f, 1.0f);
    const ImVec4 text_disabled(0.62f, 0.52f, 0.58f, 1.0f);
    const ImVec4 border(pink_mid.x, pink_mid.y, pink_mid.z, 0.6f);
    const ImVec4 separator(pink_mid.x, pink_mid.y, pink_mid.z, 0.5f);

    auto set = [&s](ImGuiCol c, const ImVec4& v) { s.Colors[c] = v; };
    set(ImGuiCol_WindowBg, bg);
    set(ImGuiCol_ChildBg, bg_child);
    set(ImGuiCol_PopupBg, bg_child);
    set(ImGuiCol_Border, border);
    set(ImGuiCol_BorderShadow, ImVec4(0.f, 0.f, 0.f, 0.35f));
    set(ImGuiCol_FrameBg, frame);
    set(ImGuiCol_FrameBgHovered, frame_hover);
    set(ImGuiCol_FrameBgActive, frame_active);
    set(ImGuiCol_TitleBg, pink_dark);
    set(ImGuiCol_TitleBgActive, pink_mid);
    set(ImGuiCol_TitleBgCollapsed, pink_dark);
    set(ImGuiCol_MenuBarBg, frame);
    set(ImGuiCol_ScrollbarBg, bg);
    set(ImGuiCol_ScrollbarGrab, frame_active);
    set(ImGuiCol_ScrollbarGrabHovered, pink_mid);
    set(ImGuiCol_ScrollbarGrabActive, pink);
    set(ImGuiCol_CheckMark, pink);
    set(ImGuiCol_CheckboxSelectedBg, frame_active);
    set(ImGuiCol_SliderGrab, pink);
    set(ImGuiCol_SliderGrabActive, pink_mid);
    set(ImGuiCol_Button, frame);
    set(ImGuiCol_ButtonHovered, frame_hover);
    set(ImGuiCol_ButtonActive, frame_active);
    set(ImGuiCol_Header, pink_dark);
    set(ImGuiCol_HeaderHovered, frame_hover);
    set(ImGuiCol_HeaderActive, frame_active);
    set(ImGuiCol_Separator, separator);
    set(ImGuiCol_SeparatorHovered, pink);
    set(ImGuiCol_SeparatorActive, pink);
    set(ImGuiCol_ResizeGrip, pink_mid);
    set(ImGuiCol_ResizeGripHovered, pink);
    set(ImGuiCol_ResizeGripActive, pink);
    set(ImGuiCol_Tab, bg_child);
    set(ImGuiCol_TabHovered, frame_hover);
    set(ImGuiCol_TabSelected, frame_active);
    set(ImGuiCol_TabSelectedOverline, pink);
    set(ImGuiCol_TabDimmed, ImVec4(bg.x, bg.y, bg.z, 0.6f));
    set(ImGuiCol_TabDimmedSelected, frame);
    set(ImGuiCol_Text, text);
    set(ImGuiCol_TextDisabled, text_disabled);
    set(ImGuiCol_TextLink, pink);
    set(ImGuiCol_TextSelectedBg, ImVec4(pink_mid.x, pink_mid.y, pink_mid.z, 0.5f));
    set(ImGuiCol_DragDropTarget, pink);
    set(ImGuiCol_NavCursor, pink);
    set(ImGuiCol_NavWindowingHighlight, pink);
    set(ImGuiCol_NavWindowingDimBg, ImVec4(0.f, 0.f, 0.f, 0.4f));
    set(ImGuiCol_ModalWindowDimBg, ImVec4(0.f, 0.f, 0.f, 0.5f));

    s.WindowRounding = 4.f;
    s.ChildRounding = 3.f;
    s.FrameRounding = 3.f;
    s.GrabRounding = 3.f;
    s.TabRounding = 3.f;
    s.ScrollbarRounding = 3.f;
    s.WindowBorderSize = 1.f;
    s.FrameBorderSize = 0.f;
    s.GrabMinSize = 8.f;

    s.ScaleAllSizes(0.85f);
}

}  // namespace

bool Overlay::create(int x, int y, int w, int h, bool topmost) {
    x_ = x; y_ = y; w_ = w; h_ = h;

    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = Overlay::wnd_proc;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.lpszClassName = L"SummerCampEspOverlay";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    RegisterClassExW(&wc);

    DWORD ex = WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE;
    const AlphaRecipe recipe = recipe_from_env();
    if (recipe != AlphaRecipe::kDwmPlain) ex |= WS_EX_LAYERED;
    if (topmost) ex |= WS_EX_TOPMOST;

    hwnd_ = CreateWindowExW(ex, wc.lpszClassName, L"", WS_POPUP, x_, y_, w_, h_,
                            nullptr, nullptr, wc.hInstance, nullptr);
    if (!hwnd_) return false;

    if (recipe == AlphaRecipe::kDwmAlphaAttr) {
        // without this the layered window ignores the swapchain alpha.
        // It must come before the DWM frame extension.
        SetLayeredWindowAttributes(hwnd_, 0, 255, LWA_ALPHA);
    }
    // per-pixel alpha: stretch the DWM frame over the whole client area
    MARGINS m{-1, -1, -1, -1};
    DwmExtendFrameIntoClientArea(hwnd_, &m);

    if (!init_d3d()) {
        DestroyWindow(hwnd_);
        hwnd_ = nullptr;
        return false;
    }

    SetWindowLongPtrW(hwnd_, GWL_EXSTYLE, ex);
    SetWindowPos(hwnd_, topmost ? HWND_TOPMOST : HWND_TOP, x_, y_, w_, h_,
                 SWP_SHOWWINDOW);
    ShowWindow(hwnd_, SW_SHOWNOACTIVATE);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO().IniFilename = nullptr;  // no imgui.ini on disk
    apply_pink_theme();

    // System faces over the pixelated ImGui default. Atlas order:
    // 0 = menu, 1..3 = ESP sizes. A Japanese face goes after them, only
    // used by the menu.
    ImFont* ui_font = ImGui::GetIO().Fonts->AddFontFromFileTTF(
        "C:\\Windows\\Fonts\\segoeui.ttf", fonts::kUi);
    ImGui::GetIO().Fonts->AddFontFromFileTTF(
        "C:\\Windows\\Fonts\\segoeui.ttf", fonts::kEspSmall);
    ImGui::GetIO().Fonts->AddFontFromFileTTF(
        "C:\\Windows\\Fonts\\segoeui.ttf", fonts::kEspMedium);
    ImGui::GetIO().Fonts->AddFontFromFileTTF(
        "C:\\Windows\\Fonts\\segoeui.ttf", fonts::kEspLarge);
    if (!ui_font) ui_font = ImGui::GetIO().Fonts->AddFontDefault();
    ImGui::GetIO().FontDefault = ui_font;

    // Japanese face for the menu: Meiryo on any Windows 10 machine, Yu
    // Gothic and MS Gothic as the older alternatives. Without one of
    // them the menu keeps the English face (i18n falls back).
    ImFont* ja_font = nullptr;
    {
        const ImWchar* ja_ranges = ImGui::GetIO().Fonts->GetGlyphRangesJapanese();
        const char* ja_paths[] = {"C:\\Windows\\Fonts\\meiryo.ttc",
                                  "C:\\Windows\\Fonts\\YuGothM.ttc",
                                  "C:\\Windows\\Fonts\\msgothic.ttc"};
        for (const char* path : ja_paths) {
            ja_font = ImGui::GetIO().Fonts->AddFontFromFileTTF(
                path, fonts::kUi, nullptr, ja_ranges);
            if (ja_font) break;
        }
    }

    i18n::g_font_en = ui_font;
    i18n::g_font_ja = ja_font ? ja_font : ui_font;

    ImGui_ImplWin32_Init(hwnd_);
    ImGui_ImplDX11_Init(device_, context_);
    return true;
}

bool Overlay::init_d3d() {
    // On MinGW DXGI requires COM to be initialized (apartment): without it
    // D3D11CreateDeviceAndSwapChain throws a C++ exception and kills the
    // process. (Under MSVC this is implicit.)
    if (!com_init_) {
        const HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        com_init_ = SUCCEEDED(hr) || hr == RPC_E_CHANGED_MODE;
    }

    // Description in the format DXGI expects for a windowed swap chain.
    // Filling Width/Height/RefreshRate with fixed values sends DXGI down
    // the mode enumeration path and the process dies in the MinGW unwinder.
    DXGI_SWAP_CHAIN_DESC sd{};
    sd.BufferCount = 2;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hwnd_;
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    D3D_FEATURE_LEVEL fl{};
    const D3D_FEATURE_LEVEL fls[2] = {D3D_FEATURE_LEVEL_11_0,
                                       D3D_FEATURE_LEVEL_10_0};
    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, fls, 2, D3D11_SDK_VERSION,
        &sd, &swapchain_, &device_, &fl, &context_);
    if (FAILED(hr)) {
        // second attempt: WARP (works with no physical GPU)
        hr = D3D11CreateDeviceAndSwapChain(
            nullptr, D3D_DRIVER_TYPE_WARP, nullptr, 0, fls, 2, D3D11_SDK_VERSION,
            &sd, &swapchain_, &device_, &fl, &context_);
    }
    if (FAILED(hr)) return false;

    ID3D11Texture2D* back = nullptr;
    if (FAILED(swapchain_->GetBuffer(0, IID_PPV_ARGS(&back)))) return false;
    hr = device_->CreateRenderTargetView(back, nullptr, &rtv_);
    back->Release();
    if (FAILED(hr)) return false;

    d3d_ready_ = true;
    return true;
}

void Overlay::release_d3d() {
    if (rtv_) { rtv_->Release(); rtv_ = nullptr; }
    if (swapchain_) { swapchain_->Release(); swapchain_ = nullptr; }
    if (context_) { context_->Release(); context_ = nullptr; }
    if (device_) { device_->Release(); device_ = nullptr; }
    if (com_init_) {
        CoUninitialize();
        com_init_ = false;
    }
    d3d_ready_ = false;
}

void Overlay::destroy() {
    if (d3d_ready_) {
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
        release_d3d();
    }
    if (hwnd_) {
        DestroyWindow(hwnd_);
        hwnd_ = nullptr;
    }
}

void Overlay::set_click_through(bool through) {
    if (!hwnd_) return;
    LONG_PTR ex = GetWindowLongPtrW(hwnd_, GWL_EXSTYLE);
    if (through) {
        ex |= kExTransparent;
        SetWindowLongPtrW(hwnd_, GWL_EXSTYLE, ex);
    } else {
        // Stays NOACTIVATE: the overlay must not steal focus from the game,
        // otherwise the keyboard stops reaching the character.
        ex &= ~kExTransparent;
        SetWindowLongPtrW(hwnd_, GWL_EXSTYLE, ex);
    }
}

void Overlay::set_topmost(bool on) {
    if (!hwnd_) return;
    SetWindowPos(hwnd_, on ? HWND_TOPMOST : HWND_TOP, x_, y_, w_, h_,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
}

void Overlay::move(int x, int y, int w, int h) {
    if (!hwnd_ || w <= 0 || h <= 0) return;
    x_ = x; y_ = y; w_ = w; h_ = h;
    SetWindowPos(hwnd_, HWND_TOPMOST, x_, y_, w_, h_,
                 SWP_NOACTIVATE | SWP_SHOWWINDOW);
}

bool Overlay::pump() {
    MSG msg;
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        if (msg.message == WM_QUIT) {
            close_ = true;
            return false;
        }
    }
    return true;
}

void Overlay::begin_frame() {
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();

    // The ImGui backend only processes the mouse while its own window is
    // in the foreground. Since the overlay never activates (and must not,
    // to avoid stealing the game's keyboard), we fill the state by hand.
    ImGuiIO& io = ImGui::GetIO();
    POINT cur{};
    if (GetCursorPos(&cur)) {
        io.MousePos = ImVec2(static_cast<float>(cur.x - x_),
                             static_cast<float>(cur.y - y_));
    }
    io.MouseDown[0] = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
    io.MouseDown[1] = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
    io.MouseDown[2] = (GetAsyncKeyState(VK_MBUTTON) & 0x8000) != 0;

    ImGui::NewFrame();
}

void Overlay::end_frame() {
    ImGui::Render();
    context_->OMSetRenderTargets(1, &rtv_, nullptr);
    const FLOAT clear[4] = {0.f, 0.f, 0.f, 0.f};
    context_->ClearRenderTargetView(rtv_, clear);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    swapchain_->Present(1, 0);  // vsync
}

LRESULT CALLBACK Overlay::wnd_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wp, lp)) return 0;
    switch (msg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        default:
            break;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}
