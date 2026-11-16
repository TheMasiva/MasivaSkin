#pragma once
#include "loadout.hpp"
#include "catalog.hpp"
#include <map>
#include <utility>
#include <set>
namespace masiva {
class Selections {
    std::map<std::pair<int,int>,Cosmetic> choices_;
    static std::pair<int,int> key(int team,int definition) {
        return {team,(definition==60000 || catalog::agent(definition))?60000:catalog::glove(definition)?5028:catalog::knife(definition)?42:definition};
    }
public:
    const Cosmetic* get(int team,int definition) const {
        auto i=choices_.find(key(team,definition));return i==choices_.end()?nullptr:&i->second;
    }
    void set(const Cosmetic& c) {
        validate(c);
        if(!catalog::finish(c.definition,c.paint) ||
            c.category!=catalog::category(c.definition))
            throw std::runtime_error("Gecersiz esya/kaplama secimi.");
        choices_[key(c.team=="T"?2:3,c.definition)]=c;
    }
    void clear(int team,int definition) { choices_.erase(key(team,definition)); }
    void set_all(const std::vector<Cosmetic>& items) {
        auto next=*this;
        std::set<std::pair<int,int>> seen;
        for(const auto& c:items) {
            validate(c);
            auto finish=catalog::finish(c.definition,c.paint);
            if(!finish || c.wear<finish->minimum || c.wear>finish->maximum)
                throw std::runtime_error("Profile finish/wear does not match the catalog");
            if(!seen.insert(key(c.team=="T"?2:3,c.definition)).second)
                throw std::runtime_error("Duplicate profile slot");
            next.set(c);
        }
        choices_.swap(next.choices_);
    }
    void clear() { choices_.clear(); }
};
}
