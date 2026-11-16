#include "loadout.hpp"
#include "release.hpp"
#include <fstream>
#include <iostream>

int main(int argc, char** argv) {
    std::cout << "Masiva Skin " << masiva::release::version << " — " << masiva::release::codename << " — profile CLI\nFor the experimental game connection use the Windows GUI.\n";
    if (argc != 2) {
        std::cout << "Usage: MasivaProfile <profile.masiva>\n";
        return argc == 1 ? 0 : 2;
    }
    try {
        std::ifstream file(argv[1]);
        if (!file) throw std::runtime_error("Cannot open profile");
        auto items = masiva::read(file);
        masiva::write(std::cout, items);
        std::cout << "Profile valid. No changes applied to a game.\n";
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
