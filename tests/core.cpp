#include "loadout.hpp"
#include <sstream>
#include <iostream>

void require(bool ok) { if (!ok) throw std::runtime_error("Test failed"); }
void reject(const std::string& input) {
    std::istringstream s(input);
    bool rejected = false;
    try { masiva::read(s); } catch (const std::runtime_error&) { rejected = true; }
    require(rejected);
}
int main() {
    std::istringstream input("MASIVA_SKIN 1 2\nT weapon 7 0 0 0.01\nCT knife 500 0 1000 1\n");
    auto items = masiva::read(input);
    std::ostringstream serialized;
    masiva::write(serialized, items);
    std::istringstream again(serialized.str());
    auto copy = masiva::read(again);
    require(copy.size() == 2 && copy[0].wear == items[0].wear && copy[1].seed == 1000);
    reject("MASIVA_SKIN 4 0");
    reject("MASIVA_SKIN 1 513");
    reject("MASIVA_SKIN 1 1\nT weapon 7 0 1001 0.1");
    reject("MASIVA_SKIN 1 1\nT weapon 7 0 0 -0.1");
    reject("MASIVA_SKIN 1 1\nT weapon 7");
    reject("MASIVA_SKIN 1 0\njunk");
    reject("MASIVA_SKIN 1 2\nT knife 500 0 0 0.1\nT knife 507 0 0 0.1");
    {
        std::ostringstream out;
        bool rejected = false;
        try { masiva::write(out, {{"T", "knife", 500, 0, 0, 0.1f}, {"T", "knife", 507, 0, 0, 0.1f}}); }
        catch (const std::runtime_error&) { rejected = true; }
        require(rejected && out.str().empty());
    }
    {
        std::ostringstream out;
        bool rejected = false;
        try { masiva::write(out, {{"T", "weapon", 7, 0, 0, NAN}}); }
        catch (const std::runtime_error&) { rejected = true; }
        require(rejected && out.str().empty());
    }
    require(copy[0].stattrak==-1 && copy[0].custom_name.empty());
    items[0].stattrak=12345;items[0].custom_name="Masiva \"Özel\" | 紫";
    std::ostringstream modern;masiva::write(modern,items);std::istringstream modern_in(modern.str());
    auto updated=masiva::read(modern_in);
    require(updated[0].stattrak==12345 && updated[0].custom_name==items[0].custom_name);
    reject("MASIVA_SKIN 2 1\nT weapon 7 0 0 0.1 -2 \"\"");
    reject("MASIVA_SKIN 2 1\nT weapon 7 0 0 0.1 1000000 \"\"");
    reject("MASIVA_SKIN 2 1\nT gloves 5030 10018 0 0.1 1 \"\"");
    reject("MASIVA_SKIN 2 1\nT weapon 7 0 0 0.1 0");
    reject("MASIVA_SKIN 2 1\nT weapon 7 0 0 0.1 -1 \""+std::string(161,'a')+"\"");
    std::cout << "Profile v1 compatibility and v2 StatTrak/name tag checks passed\n";
}
