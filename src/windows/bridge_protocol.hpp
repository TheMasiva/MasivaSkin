#pragma once
#include <windows.h>
#include <cstdint>
#include <string>
#include "../profile_packet.hpp"
namespace masiva::bridge {
inline constexpr DWORD version = 91202; // Internal IPC revision; public release remains 0.9.12 Asuna.
enum State : LONG { Idle, Queued, Running, Done };
enum Operation : DWORD { Apply = 1, Restore = 2, Stop = 3, RestoreGloves = 4, Queue = 5, QueueProfile = 6, RestoreAgent = 7, ClearBoth = 8, QueueBoth = 9, RestoreAll = 10 };
// One producer, one consumer. Fields are published by InterlockedExchange;
// neither side changes a payload while the other owns it.
struct alignas(8) Mailbox {
    DWORD magic, protocol, pid;
    volatile LONG initialized;
    volatile LONG state;
    volatile LONG heartbeat;
    volatile LONG active_backup;
    volatile LONG glove_backup;
    volatile LONG glove_updates;
    char glove_message[256];
    char glove_details[384];
    volatile LONG part_updates;
    char part_message[256];
    volatile LONG hud_updates;
    char hud_message[256];
    volatile LONG pending_backups;
    DWORD operation, team, definition;
    volatile LONG auto_updates;
    char auto_message[256];
    int paint, seed;
    float wear;
    int stattrak;
    char custom_name[161];
    DWORD request_id, completed_id, success, frame_thread, stage;
    std::uint64_t deadline, weapon, hud, expected_pawn, expected_identity;
    DWORD expected_handle;
    int effective_paint, effective_seed;
    float effective_wear;
    int world_materials, hud_materials;
    char message[512];
    ProfilePacket profile;
    Decorations decorations;
    char agent_message[256];
    char agent_details[512];
    volatile LONG agent_requests,material_reuses,hud_row_rebuilds;
    volatile LONG death_events,death_icons;
    char death_message[256];
    volatile LONG voice_ready,voice_local_events,voice_matches,voice_replacements,voice_errors;
    char voice_message[512];
    volatile LONG voice_history_count;
    char voice_history[12][256];
    volatile LONG agent_backup;
    volatile LONG full_hud_refresh;
    volatile LONG startup_stage;
    volatile LONG glove_backups,glove_pruned;
    volatile LONG preview_ready,preview_updates,preview_errors,preview_local_calls;
    char preview_message[256];
    volatile LONG preview_ticks,preview_thread_skips,preview_busy_skips,preview_nonlocal;
    std::uint64_t preview_work_us;
    volatile LONG preview_work_calls,preview_max_us,preview_idle_skips,preview_item_skips;
    char hud_language[32];
    std::uint64_t frame_work_us,hud_work_us;
    volatile LONG frame_work_calls,hud_work_calls,frame_max_us,hud_max_us;
    volatile LONG hud_skipped,watch_checks_skipped,weapon_backups_pruned,glove_recoveries;
};
inline constexpr DWORD magic = 0x4d534b33;
inline std::wstring name(DWORD pid) { return L"Local\\MasivaSkin-098-" + std::to_wstring(pid); }
}
