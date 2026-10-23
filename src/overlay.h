// overlay.h — transparent window on top of the game (Win32 + D3D11 + ImGui).
#pragma once

#include <windows.h>
#include <d3d11.h>

#include <functional>
#include <string>

class Overlay {
public:
    Overlay() = default;
    ~Overlay() { destroy(); }
    Overlay(const Overlay&) = delete;
    Overlay& operator=(const Overlay&) = delete;

    // Creates the window over the game area. Returns false on failure.
    bool create(int x, int y, int w, int h, bool topmost);
    void destroy();

    // Pumps Windows messages. Returns false if the window died.
    bool pump();

    void begin_frame();
    void end_frame();

    // Toggles click-through: with the menu closed the game gets the
    // mouse as usual.
    void set_click_through(bool through);
    void set_topmost(bool on);

    // Repositions and resizes the overlay window.
    void move(int x, int y, int w, int h);

    bool should_close() const { return close_; }
    int width() const { return w_; }
    int height() const { return h_; }
    HWND hwnd() const { return hwnd_; }

private:
    static LRESULT CALLBACK wnd_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    bool init_d3d();
    void release_d3d();

    HWND hwnd_ = nullptr;
    ID3D11Device* device_ = nullptr;
    ID3D11DeviceContext* context_ = nullptr;
    IDXGISwapChain* swapchain_ = nullptr;
    ID3D11RenderTargetView* rtv_ = nullptr;

    int x_ = 0, y_ = 0, w_ = 1920, h_ = 1080;
    bool close_ = false;
    bool d3d_ready_ = false;
    bool com_init_ = false;
};
