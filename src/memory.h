// memory.h — passive reading of the game process.
//
// We open the process read-only (PROCESS_VM_READ) and never write.
// No injection, no driver, no writes into the game.
#pragma once

#include <windows.h>
#include <psapi.h>

#include <cstdint>
#include <cstring>
#include <string>

namespace mem {

class Process {
public:
    Process() = default;
    ~Process() { close(); }
    Process(const Process&) = delete;
    Process& operator=(const Process&) = delete;

    // Finds the game process: the module must fit the GWorld offset and the
    // GWorld->camera chain must yield a plausible FOV.
    bool attach(const char* module_hint);
    void close();

    bool ok() const { return handle_ != nullptr; }
    HANDLE handle() const { return handle_; }
    uintptr_t base() const { return base_; }
    const std::string& path() const { return path_; }
    DWORD pid() const { return pid_; }

    bool read(uintptr_t addr, void* dst, size_t size) const {
        if (!handle_ || !addr) return false;
        SIZE_T got = 0;
        return ReadProcessMemory(handle_, reinterpret_cast<LPCVOID>(addr), dst,
                                 size, &got) != FALSE &&
               got == size;
    }

    template <typename T>
    T read(uintptr_t addr, T fallback = T{}) const {
        T v = fallback;
        if (addr) read(addr, &v, sizeof(T));
        return v;
    }

    template <typename T>
    bool read(uintptr_t addr, T* dst) const {
        return read(addr, dst, sizeof(T));
    }

private:
    HANDLE handle_ = nullptr;
    uintptr_t base_ = 0;
    DWORD pid_ = 0;
    std::string path_;
};

}  // namespace mem
