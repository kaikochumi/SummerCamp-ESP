// memory.cpp — game process locating and reading.
#include "memory.h"

#include <algorithm>
#include <cctype>
#include <vector>

#include "offsets.h"

namespace mem {
namespace {

bool plausible_ptr(uintptr_t p) {
    return p > 0x10000 && p < 0x7FFFFFFFFFFFull;
}

template <typename T>
T read_at(HANDLE h, uintptr_t addr, T fallback = T{}) {
    T v = fallback;
    if (!h || !addr) return fallback;
    SIZE_T got = 0;
    if (ReadProcessMemory(h, reinterpret_cast<LPCVOID>(addr), &v, sizeof(T), &got) &&
        got == sizeof(T))
        return v;
    return fallback;
}

// The GWorld offset must fit in the module AND the GWorld->camera chain
// must close. This is what separates the game from the launcher.
bool chain_ok(HANDLE h, uintptr_t base) {
    if (!base) return false;
    const uintptr_t world = read_at<uintptr_t>(h, base + Offsets::GWorld);
    if (!plausible_ptr(world)) return false;
    const uintptr_t level = read_at<uintptr_t>(h, world + Offsets::OFF_WORLD_LEVEL);
    if (!plausible_ptr(level)) return false;
    const uint32_t count = read_at<uint32_t>(h, level + Offsets::OFF_LEVEL_ACTORS + 8);
    if (count == 0 || count > 200000) return false;

    const uintptr_t gi = read_at<uintptr_t>(h, world + Offsets::OFF_WORLD_GI);
    const uintptr_t lps = read_at<uintptr_t>(h, gi + Offsets::OFF_GI_LOCALPLAYERS);
    const uintptr_t lp = read_at<uintptr_t>(h, lps);
    const uintptr_t pc = read_at<uintptr_t>(h, lp + Offsets::OFF_PLAYER_PC);
    const uintptr_t cam = read_at<uintptr_t>(h, pc + Offsets::OFF_PC_CAMMGR);
    if (!plausible_ptr(cam)) return false;

    const uintptr_t pov = cam + Offsets::OFF_CAM_CACHE + Offsets::OFF_CACHE_POV;
    const float fov = read_at<float>(h, pov + Offsets::OFF_POV_FOV);
    return fov > 1.f && fov < 179.f;
}

}  // namespace

void Process::close() {
    if (handle_) {
        CloseHandle(handle_);
        handle_ = nullptr;
    }
    base_ = 0;
    pid_ = 0;
    path_.clear();
}

bool Process::attach(const char* module_hint) {
    close();

    DWORD pids[4096];
    DWORD bytes = 0;
    if (!EnumProcesses(pids, sizeof(pids), &bytes)) return false;
    const DWORD count = bytes / sizeof(DWORD);

    for (DWORD i = 0; i < count; ++i) {
        const DWORD pid = pids[i];
        if (!pid) continue;

        HANDLE h = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, pid);
        if (!h) continue;

        HMODULE mods[1024];
        DWORD needed = 0;
        if (!EnumProcessModulesEx(h, mods, sizeof(mods), &needed, LIST_MODULES_ALL)) {
            CloseHandle(h);
            continue;
        }

        const DWORD nmods = needed / sizeof(HMODULE);
        // only the main module (the .exe). The game's GWorld lives in the
        // executable; scanning all DLLs is slow and finds nothing new.
        for (DWORD m = 0; m < nmods && m < 1; ++m) {
            MODULEINFO mi{};
            if (!GetModuleInformation(h, mods[m], &mi, sizeof(mi))) continue;
            if (mi.SizeOfImage <= Offsets::GWorld + 0x1000) continue;

            wchar_t wpath[MAX_PATH]{};
            GetModuleFileNameExW(h, mods[m], wpath, MAX_PATH);
            std::string spath;
            for (const wchar_t* w = wpath; *w; ++w) {
                const char c = static_cast<char>(*w);
                spath.push_back(c >= 'A' && c <= 'Z' ? c + 32 : c);
            }

            const bool chain = chain_ok(h, reinterpret_cast<uintptr_t>(mi.lpBaseOfDll));
            if (getenv("ESP_DEBUG"))
                fprintf(stderr, "[scan] pid=%lu size=%zu chain=%d %s\n", pid,
                        mi.SizeOfImage, chain ? 1 : 0, spath.c_str());
            if (!chain) continue;
            if (module_hint && *module_hint) {
                std::string hint;
                for (const char* c = module_hint; *c; ++c)
                    hint.push_back(static_cast<char>(std::tolower(*c)));
                if (spath.find(hint) == std::string::npos) continue;
            }

            handle_ = h;
            base_ = reinterpret_cast<uintptr_t>(mi.lpBaseOfDll);
            pid_ = pid;
            path_ = spath;
            return true;
        }
        CloseHandle(h);
    }
    return false;
}

}  // namespace mem
