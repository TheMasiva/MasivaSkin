#pragma once
#include <windows.h>
#include <string>
namespace masiva::rpc {
void settings(HWND parent);
bool is_enabled();
void set_enabled(bool value);
void game_started(const std::wstring& exe);
void stop();
}
