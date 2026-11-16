#pragma once
#include <stdexcept>
#include <string>
#include <string_view>

namespace masiva {
inline int parse_integer(std::string_view value, const char* field, int maximum, bool empty_is_zero = false) {
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string_view::npos) {
        if (empty_is_zero) return 0;
        throw std::runtime_error(std::string(field) + " bos birakilamaz.");
    }
    value = value.substr(first, value.find_last_not_of(" \t\r\n") - first + 1);
    int result = 0;
    for (const unsigned char c : value) {
        if (c < '0' || c > '9')
            throw std::runtime_error(std::string(field) + " tam sayi olmali (ornek: 0 veya 42). Float ayri alana yazilir.");
        if (result > maximum / 10 || (result == maximum / 10 && c - '0' > maximum % 10))
            throw std::runtime_error(std::string(field) + " 0-" + std::to_string(maximum) + " araliginda olmali.");
        result = result * 10 + c - '0';
    }
    return result;
}
}
