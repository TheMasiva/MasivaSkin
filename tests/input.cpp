#include "input.hpp"
#include <iostream>

void require(bool v) { if (!v) throw std::runtime_error("input test failed"); }
void reject(const char* s) {
    bool bad = false;
    try { masiva::parse_integer(s, "Seed", 1000, true); }
    catch (const std::runtime_error&) { bad = true; }
    require(bad);
}
int main() {
    require(masiva::parse_integer("", "Seed", 1000, true) == 0);
    require(masiva::parse_integer(" \t42\r\n", "Seed", 1000, true) == 42);
    require(masiva::parse_integer("1000", "Seed", 1000, true) == 1000);
    reject("1001"); reject("-1"); reject("0.01"); reject("0,01");
    reject("12x"); reject("999999999999999999999999999999"); reject("4 2");
    std::cout << "10 input checks passed\n";
}
