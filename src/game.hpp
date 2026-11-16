#pragma once
#include "loadout.hpp"
#include "offsets.hpp"
#include "catalog.hpp"
#include <array>
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <optional>
#include <type_traits>

namespace masiva {
using Address = std::uint64_t;
class Memory {
public:
    virtual ~Memory() = default;
    virtual bool alive() const = 0;
    virtual void read_bytes(Address, void*, std::size_t) const = 0;
    virtual void write_bytes(Address, const void*, std::size_t) = 0;
    template<class T> T get(Address a) const {
        static_assert(std::is_trivially_copyable_v<T>);
        T v{}; read_bytes(a, &v, sizeof(v)); return v;
    }
    template<class T> void put(Address a, const T& v) {
        static_assert(std::is_trivially_copyable_v<T>);
        write_bytes(a, &v, sizeof(v));
    }
};
inline Address pointer(Address p) {
    if (p < 0x10000 || p > 0x00007ffffffeffffULL || p % 8)
        throw std::runtime_error("Gecersiz oyun adresi; baglanti yenilenmeli.");
    return p;
}
struct WeaponState {
    Address pawn{}, weapon{}, identity{}, item{};
    std::uint32_t handle{};
    int team{}, health{};
    std::uint16_t definition{};
    bool same_object(const WeaponState& b) const {
        return pawn == b.pawn && weapon == b.weapon && identity == b.identity &&
               item == b.item && handle == b.handle && team == b.team;
    }
    bool same_entity(const WeaponState& b) const { return same_object(b) && definition==b.definition; }
};
struct FallbackValues {
    std::int32_t paint{}, seed{};
    float wear{};
    std::uint32_t item_id_high{};
    bool operator==(const FallbackValues& b) const {
        return paint == b.paint && seed == b.seed && wear == b.wear && item_id_high == b.item_id_high;
    }
};
class Game {
    Memory& memory_;
    Address client_, engine_;
    int expected_build_;
    struct Backup { WeaponState state; FallbackValues original, last; };
    std::vector<Backup> backups_;
    std::optional<std::size_t> backup_index(const WeaponState& state) const {
        for (std::size_t i = 0; i < backups_.size(); ++i)
            if (backups_[i].state.same_entity(state) || (backups_[i].state.same_object(state) &&
                catalog::knife(backups_[i].state.definition) && catalog::knife(state.definition))) return i;
        return std::nullopt;
    }
    static FallbackValues restore_owned(const FallbackValues& current, const Backup& backup) {
        // Preserve fields changed by the game since our last write.
        auto result = current;
        if (current.paint == backup.last.paint) result.paint = backup.original.paint;
        if (current.seed == backup.last.seed) result.seed = backup.original.seed;
        if (current.wear == backup.last.wear) result.wear = backup.original.wear;
        if (current.item_id_high == backup.last.item_id_high) result.item_id_high = backup.original.item_id_high;
        return result;
    }

    Address entity(std::uint32_t handle, Address* slot = nullptr) const {
        if (handle == 0xffffffffU || (handle & 0x7fffU) == 0)
            throw std::runtime_error("Aktif silah bulunamadi.");
        const auto list = pointer(memory_.get<Address>(client_ + offsets::entity_list));
        const auto chunk = pointer(memory_.get<Address>(list + 0x10 + 8 * ((handle & 0x7fffU) >> 9)));
        const auto entry = chunk + offsets::entity_stride * (handle & 0x1ffU);
        if (slot) *slot = entry;
        return pointer(memory_.get<Address>(entry));
    }
    void guard(const WeaponState& expected) const {
        if (!snapshot().same_entity(expected) || !owned_weapon(expected))
            throw std::runtime_error("Silah/oyuncu degisti; islem durduruldu.");
    }
    void set_values(const WeaponState& s, const FallbackValues& v) {
        guard(s); memory_.put(s.weapon + offsets::paint, v.paint);
        guard(s); memory_.put(s.weapon + offsets::seed, v.seed);
        guard(s); memory_.put(s.weapon + offsets::wear, v.wear);
        // Switch to fallback only after the data fields have been written.
        guard(s); memory_.put(s.item + offsets::item_id_high, v.item_id_high);
        guard(s);
        if (!(values(s) == v)) throw std::runtime_error("Yazilan degerler geri okumada eslesmedi.");
    }
public:
    Game(Memory& memory, Address client, Address engine, int verified_build)
        : memory_(memory), client_(pointer(client)), engine_(pointer(engine)), expected_build_(verified_build) {
        if(verified_build<=0)throw std::runtime_error("Validated session build required.");
    }
    int expected_build() const {return expected_build_;}
    int build() const {
        if (!memory_.alive()) throw std::runtime_error("CS2 kapandi. Yeniden baglan.");
        return memory_.get<int>(engine_ + offsets::build_number);
    }
    WeaponState player_snapshot() const {
        if (build() != expected_build_)
            throw std::runtime_error("CS2 surumu desteklenen offset paketiyle eslesmiyor.");
        WeaponState s;
        s.pawn = pointer(memory_.get<Address>(client_ + offsets::local_pawn));
        s.health = memory_.get<int>(s.pawn + offsets::health);
        s.team = memory_.get<std::uint8_t>(s.pawn + offsets::team);
        if (s.health <= 0 || s.health > 100 || memory_.get<std::uint8_t>(s.pawn + offsets::life_state) != 0 ||
            (s.team != 2 && s.team != 3))
            throw std::runtime_error("Canli oyuncu bekleniyor; menude/izleyicide veya olusun.");
        return s;
    }
    WeaponState glove_snapshot() const {
        auto s=player_snapshot();
        s.weapon=s.pawn;s.item=s.pawn+0x17a0;
        s.identity=pointer(memory_.get<Address>(s.pawn+offsets::identity));
        s.handle=memory_.get<std::uint32_t>(s.identity+0x10);
        s.definition=memory_.get<std::uint16_t>(s.item+offsets::definition);
        return s;
    }
    WeaponState snapshot() const {
        auto s=player_snapshot();
        const auto services = pointer(memory_.get<Address>(s.pawn + offsets::weapon_services));
        s.handle = memory_.get<std::uint32_t>(services + offsets::active_weapon);
        Address slot{};
        s.weapon = entity(s.handle, &slot);
        s.identity = pointer(memory_.get<Address>(s.weapon + offsets::identity));
        if (s.identity != slot)
            throw std::runtime_error("Entity yerlesimi dogrulanamadi; yazma engellendi.");
        const auto owner = memory_.get<std::uint32_t>(s.weapon + offsets::owner);
        if (entity(owner) != s.pawn)
            throw std::runtime_error("Aktif silahin sahibi yerel oyuncu degil.");
        s.item = s.weapon + offsets::attribute_manager + offsets::item;
        s.definition = memory_.get<std::uint16_t>(s.item + offsets::definition);
        if (s.definition == 0 || s.definition > 65500)
            throw std::runtime_error("Silah tanimi okunamadi.");
        // Re-check the handle at the end of this multi-read snapshot.
        if (memory_.get<std::uint32_t>(services + offsets::active_weapon) != s.handle ||
            memory_.get<Address>(client_ + offsets::local_pawn) != s.pawn)
            throw std::runtime_error("Okuma sirasinda oyuncu veya silah degisti.");
        return s;
    }
    // The current holder is not necessarily the original owner (dropped weapons).
    // Unknown ownership is preserved rather than guessed from the item account ID.
    bool owned_weapon(const WeaponState& s) const noexcept {
        try {
            if(entity(s.handle)!=s.weapon || memory_.get<Address>(s.weapon+offsets::identity)!=s.identity ||
                entity(memory_.get<std::uint32_t>(s.weapon+offsets::owner))!=s.pawn)return false;
            const auto controller=entity(memory_.get<std::uint32_t>(s.pawn+offsets::pawn_controller));
            if(entity(memory_.get<std::uint32_t>(controller+offsets::controller_pawn))!=s.pawn)return false;
            const auto local_id=memory_.get<std::uint64_t>(controller+offsets::controller_steam_id);
            return local_id!=0 && memory_.get<std::uint64_t>(s.weapon+offsets::original_owner)==local_id;
        } catch(...) {return false;}
    }
    FallbackValues values(const WeaponState& s) const {
        FallbackValues v;
        v.paint = memory_.get<std::int32_t>(s.weapon + offsets::paint);
        v.seed = memory_.get<std::int32_t>(s.weapon + offsets::seed);
        v.wear = memory_.get<float>(s.weapon + offsets::wear);
        v.item_id_high = memory_.get<std::uint32_t>(s.item + offsets::item_id_high);
        if (!std::isfinite(v.wear)) throw std::runtime_error("Oyun verisi tutarsiz.");
        return v;
    }
    // Only used for HUD presentation; never selects or modifies a holstered item.
    WeaponState held_weapon(std::uint32_t handle) const {
        auto s=player_snapshot();s.handle=handle;
        Address slot{};s.weapon=entity(handle,&slot);
        s.identity=pointer(memory_.get<Address>(s.weapon+offsets::identity));
        if(s.identity!=slot || memory_.get<std::uint32_t>(slot+0x10)!=handle ||
           entity(memory_.get<std::uint32_t>(s.weapon+offsets::owner))!=s.pawn)
            throw std::runtime_error("HUD weapon lifetime or owner changed");
        s.item=s.weapon+offsets::attribute_manager+offsets::item;
        s.definition=memory_.get<std::uint16_t>(s.item+offsets::definition);
        return s;
    }
    template<class Predicate> void prune_expired(Predicate expired) {
        backups_.erase(std::remove_if(backups_.begin(),backups_.end(),
            [&](const Backup& backup){return expired(backup.state);}),backups_.end());
    }
    bool has_backup() const { return !backups_.empty(); }
    std::size_t backup_count() const { return backups_.size(); }
    bool has_active_backup() const {
        try { return backup_index(snapshot()).has_value(); }
        catch (...) { return false; }
    }
    void apply(const Cosmetic& c) {
        validate(c);
        if (!((c.category == "weapon" && catalog::firearm(c.definition)) || (c.category == "knife" && catalog::knife(c.definition))))
            throw std::runtime_error("Yalnizca katalog silahlari ve bicaklari desteklenir.");
        const auto s = snapshot();
        if (!owned_weapon(s))throw std::runtime_error("Baska oyuncunun veya sahibi bilinmeyen silahin gorunumu korunuyor.");
        if (!catalog::compatible(s.definition,c.definition))
            throw std::runtime_error("Secilen silahi veya bicagi eline al.");
        if (s.team != (c.team == "T" ? 2 : 3)) throw std::runtime_error("Profil takimi oyuncuyla eslesmiyor.");
        const auto before = values(s);
        const auto index = backup_index(s);
        if (!index && backups_.size() >= 256)
            throw std::runtime_error("256 bekleyen silah yedegi var. Oyun oturumunu yeniden baslat.");
        FallbackValues desired{c.paint, c.seed, c.wear, 0xffffffffU};
        const std::optional<Backup> saved = index ? std::optional<Backup>{backups_[*index]} : std::nullopt;
        const std::size_t at = index ? *index : backups_.size();
        Backup next{s, saved ? restore_owned(before, *saved) : before, before};
        if (index) backups_[at] = next; else backups_.push_back(next);
        try {
            set_values(s, desired);
            backups_[at].last = desired;
        } catch (const std::exception& error) {
            const std::string why = error.what();
            try {
                set_values(s, before);
                if (saved) backups_[at] = *saved; else backups_.erase(backups_.begin() + at);
            }
            catch (...) {
                // Never hide a failed rollback. Keep the backup for diagnostics.
                throw std::runtime_error(why + " Geri alma da basarisiz; oyun oturumunu yeniden baslat.");
            }
            throw std::runtime_error(why + " Onceki degerler geri yuklendi.");
        }
    }
    void restore() {
        const auto s = snapshot();
        const auto index = backup_index(s);
        if (!index) throw std::runtime_error("Aktif silah icin geri alinacak degisiklik yok.");
        const auto current = values(s);
        const auto target = restore_owned(current, backups_[*index]);
        if (!(current == target)) set_values(s, target);
        backups_.erase(backups_.begin() + *index);
    }
};
}
