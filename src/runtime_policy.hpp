#pragma once
#include <cstdint>
#include <string>
namespace masiva {
// Cache the intended presentation, never treat a cached address as proof of life.
// Callers validate the row/pawn/owner on every pass before consulting this memo.
template<class Stamp>
class HudWriteCache {
    Stamp stamp_{};
    std::uint64_t refresh_at_{};
    bool valid_{};
public:
    bool needs(const Stamp& stamp,std::uint64_t now)const {
        return !valid_ || !(stamp==stamp_) || now>=refresh_at_;
    }
    void committed(const Stamp& stamp,std::uint64_t now) {
        stamp_=stamp;refresh_at_=now+2000;valid_=true;
    }
};
using HudWriteMemo=HudWriteCache<std::string>;
// A vanished world entity needs a fresh native request, not a repeated consumer
// update. Existing (including still loading) entities must never be restarted.
inline bool restart_missing_world(bool exists,unsigned attempts) {
    return !exists && attempts<=3;
}
}
