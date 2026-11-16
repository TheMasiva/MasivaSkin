#pragma once
#include <cstdint>
namespace masiva {
// A resource identity, not a permanent raw pointer: invalidation is required
// whenever no eligible active weapon can be observed (drop/death/menu/switch).
struct AppearanceKey {
    std::uint64_t pawn{}, weapon{}, identity{}, hud{}, world_node{}, hud_node{}, world_model{}, hud_model{};
    std::uint32_t handle{};
    int team{}, definition{};
    bool operator==(const AppearanceKey& b) const {
        return pawn==b.pawn && weapon==b.weapon && identity==b.identity && hud==b.hud &&
            world_node==b.world_node && hud_node==b.hud_node && world_model==b.world_model &&
            hud_model==b.hud_model && handle==b.handle && team==b.team && definition==b.definition;
    }
};
// Each successfully applied weapon owns a memo. Switching to another weapon
// must not invalidate the first weapon's unchanged material resources.
class AppearanceMemo {
    AppearanceKey key_{};bool valid_{};
public:
    void remember(const AppearanceKey& key){key_=key;valid_=true;}
    bool reusable(const AppearanceKey& key,bool values_match,bool compatibility)const {
        return !compatibility && values_match && valid_ && key_==key;
    }
};
class Retention {
    AppearanceKey last_{};
    bool valid_{}, observed_{};
    AppearanceKey observed_key_{};
    unsigned failures_{};
    std::uint64_t retry_at_{};
public:
    void invalidate() { valid_=false; observed_=false; failures_=0; retry_at_=0; }
    bool needs(const AppearanceKey& k,bool values_match,std::uint64_t now) {
        if(!observed_ || !(observed_key_==k)) { observed_key_=k; observed_=true; failures_=0; retry_at_=0; }
        return failures_<8 && now>=retry_at_ && (!valid_ || !(last_==k) || !values_match);
    }
    void applied(const AppearanceKey& k,std::uint64_t now) { last_=k; observed_key_=k; observed_=true; valid_=true; failures_=0; retry_at_=now+250; }
    void failed(std::uint64_t now) { ++failures_; retry_at_=now+1000; }
    // A model that has not spawned yet is not a failed cosmetic transaction.
    void waiting_for_model(std::uint64_t now) { retry_at_=now+150; }
    unsigned failures() const { return failures_; }
};
// Valve CUtlStringToken hashing for decimal item definitions.
inline std::uint32_t subclass_hash(int definition) {
    char bytes[12]{}; unsigned n=0; unsigned v=static_cast<unsigned>(definition);
    do { bytes[n++]=static_cast<char>('0'+v%10); v/=10; } while(v);
    for(unsigned i=0;i<n/2;++i) { auto c=bytes[i]; bytes[i]=bytes[n-1-i]; bytes[n-1-i]=c; }
    constexpr std::uint32_t m=0x5bd1e995;
    std::uint32_t h=0x31415926U^n; unsigned at=0,left=n;
    while(left>=4) {
        std::uint32_t k=static_cast<unsigned char>(bytes[at]) | (static_cast<unsigned char>(bytes[at+1])<<8) |
            (static_cast<unsigned char>(bytes[at+2])<<16) | (static_cast<unsigned char>(bytes[at+3])<<24);
        k*=m; k^=k>>24; k*=m; h*=m; h^=k; at+=4;left-=4;
    }
    if(left==3)h^=static_cast<unsigned char>(bytes[at+2])<<16;
    if(left>=2)h^=static_cast<unsigned char>(bytes[at+1])<<8;
    if(left>=1){h^=static_cast<unsigned char>(bytes[at]);h*=m;}
    h^=h>>13;h*=m;h^=h>>15;return h;
}
}
