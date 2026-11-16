#pragma once
#include <cstdint>
#include <cstring>
namespace masiva {
// Integer schema attributes travel through a float ABI as raw bits.
inline float integer_attribute(std::uint32_t value) {
    float result; static_assert(sizeof(result)==sizeof(value));
    std::memcpy(&result,&value,sizeof(result)); return result;
}
// Two delayed commits per changed/user-requested selection. Consume before
// execution: even a failed commit cannot create an unlimited refresh loop.
class MaterialCommit {
    std::uint64_t started_{};
    unsigned remaining_{};
public:
    void arm(std::uint64_t now) { started_=now; remaining_=2; }
    bool due(std::uint64_t now) {
        if(!remaining_)return false;
        if(now-started_>5000){remaining_=0;return false;}
        const auto delay=remaining_==2?400U:1200U;
        if(now-started_<delay)return false;
        --remaining_;return true;
    }
};
// A load window retries only the dependent first-person model, never the
// world model that is currently loading. Calls are bounded and throttled.
class ModelSettle {
    std::uint64_t deadline_{}, next_{}, next_world_{};
    unsigned world_attempts_{};
public:
    explicit ModelSettle(std::uint64_t now):deadline_(now+5000),next_(now){}
    bool world_due(bool exists,std::uint64_t now) {
        if(exists || expired(now) || now<next_world_)return false;
        next_world_=now+500;++world_attempts_;return true;
    }
    unsigned world_attempts() const { return world_attempts_; }
    bool expired(std::uint64_t now) const { return now>=deadline_; }
    bool due(std::uint64_t now) {
        if(expired(now) || now<next_)return false;
        next_=now+100; return true;
    }
};
}
