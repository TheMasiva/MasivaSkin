#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <tlhelp32.h>
#include "../game.hpp"
#include <utility>

namespace masiva {
inline std::runtime_error win_error(const char* operation) {
    return std::runtime_error(std::string(operation) + " (Windows hata " + std::to_string(GetLastError()) + ")");
}
class Handle {
    HANDLE value_ = nullptr;
public:
    explicit Handle(HANDLE h = nullptr) : value_(h) {}
    ~Handle() { if (valid()) CloseHandle(value_); }
    Handle(const Handle&) = delete;
    Handle& operator=(const Handle&) = delete;
    Handle(Handle&& h) noexcept : value_(std::exchange(h.value_, nullptr)) {}
    Handle& operator=(Handle&& h) noexcept {
        if (this != &h) { if (valid()) CloseHandle(value_); value_ = std::exchange(h.value_, nullptr); }
        return *this;
    }
    bool valid() const { return value_ && value_ != INVALID_HANDLE_VALUE; }
    HANDLE get() const { return value_; }
};
class Process final : public Memory {
    Handle process_;
    DWORD pid_{};
    std::wstring path_;
public:
    Address client{}, engine{};
    static DWORD find_pid() {
        DWORD found{};
        Handle snapshot(CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0));
        if (!snapshot.valid()) throw win_error("Islem listesi alinamadi");
        PROCESSENTRY32W entry{}; entry.dwSize = sizeof(entry);
        if (!Process32FirstW(snapshot.get(), &entry)) throw win_error("Islem listesi okunamadi");
        do {
            if (_wcsicmp(entry.szExeFile, L"cs2.exe") == 0) {
                if (found) throw std::runtime_error("Birden fazla cs2.exe acik. Tek oyun oturumu birak.");
                found = entry.th32ProcessID;
            }
        } while (Process32NextW(snapshot.get(), &entry));
        return found;
    }
    Process() {
        static_assert(sizeof(void*) == 8, "Build Masiva Skin as x64");
        pid_=find_pid();
        if(!pid_)throw std::runtime_error("CS2 bulunamadi. Once oyunu ac.");
        process_ = Handle(OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ | SYNCHRONIZE, FALSE, pid_));
        if (!process_.valid()) throw win_error("CS2 okunamadi");
        std::wstring path(32768, L'\0'); DWORD length = static_cast<DWORD>(path.size());
        if (!QueryFullProcessImageNameW(process_.get(), 0, path.data(), &length)) throw win_error("CS2 yolu okunamadi");
        path.resize(length); path_ = std::move(path);
        Handle modules;
        for (int retry = 0; retry != 3; ++retry) {
            modules = Handle(CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid_));
            if (modules.valid() || GetLastError() != ERROR_BAD_LENGTH) break;
        }
        if (!modules.valid()) throw win_error("Moduller okunamadi");
        MODULEENTRY32W module{}; module.dwSize = sizeof(module);
        if (!Module32FirstW(modules.get(), &module)) throw win_error("Moduller bulunamadi");
        do {
            if (_wcsicmp(module.szModule, L"client.dll") == 0) client = reinterpret_cast<Address>(module.modBaseAddr);
            if (_wcsicmp(module.szModule, L"engine2.dll") == 0) engine = reinterpret_cast<Address>(module.modBaseAddr);
        } while (Module32NextW(modules.get(), &module));
        if (!client || !engine) throw std::runtime_error("CS2 henuz yukleniyor; tekrar baglan.");
    }
    DWORD pid() const { return pid_; }
    const std::wstring& path() const { return path_; }
    bool alive() const override { return process_.valid() && WaitForSingleObject(process_.get(), 0) == WAIT_TIMEOUT; }
    void read_bytes(Address address, void* dest, std::size_t size) const override {
        if (!alive()) throw std::runtime_error("CS2 kapandi.");
        SIZE_T done{};
        if (!ReadProcessMemory(process_.get(), reinterpret_cast<LPCVOID>(address), dest, size, &done) || done != size)
            throw win_error("Oyun bellegi okunamadi");
    }
    void write_bytes(Address address, const void* source, std::size_t size) override {
        if (!alive()) throw std::runtime_error("CS2 kapandi.");
        // Acquire write access only for an explicit apply/restore operation.
        Handle writer(OpenProcess(PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, pid_));
        if (!writer.valid()) throw win_error("Yazma erisimi alinamadi");
        if (!alive()) throw std::runtime_error("CS2 kapandi.");
        SIZE_T done{};
        if (!WriteProcessMemory(writer.get(), reinterpret_cast<LPVOID>(address), source, size, &done) || done != size)
            throw win_error("Oyun bellegine yazilamadi");
    }
};
}
