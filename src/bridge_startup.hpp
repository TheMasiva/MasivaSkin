#pragma once
namespace masiva::startup {
enum class State { loading, initializing, ready, rejected, load_failed, missing_channel };
inline State observe(bool mapped,int initialized,bool loader_completed,bool module_present) {
    if(mapped)return initialized<0?State::rejected:initialized==1?State::ready:State::initializing;
    if(!loader_completed)return State::loading;
    return module_present?State::missing_channel:State::load_failed;
}
}
