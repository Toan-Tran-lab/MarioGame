#pragma once
#include "raylib.h"
#include <string>
#include <unordered_map>

// ---------------------------------------------------------------------------
// Audio key constants — use these instead of raw strings at call sites.
// ---------------------------------------------------------------------------
namespace AudioKey {
    // SFX
    inline constexpr const char* JUMP_SMALL       = "jump_small";
    inline constexpr const char* JUMP_SUPER        = "jump_super";
    inline constexpr const char* HIT_COIN          = "hit_coin";
    inline constexpr const char* HIT_ENEMY         = "hit_enemy";
    inline constexpr const char* MARIO_DIE         = "mario_die";
    inline constexpr const char* PIPE_TRAVEL       = "pipe_travel";
    inline constexpr const char* POWER_DOWN        = "power_down";
    inline constexpr const char* POWER_UP          = "power_up";
    inline constexpr const char* POWERUP_APPEARS   = "powerup_appears";
    inline constexpr const char* DOWN_FLAG_POLE    = "down_flag_pole";
    inline constexpr const char* FIREBALL          = "fireball";
    inline constexpr const char* STARMAN           = "starman";

    // BGM — one per level
    inline constexpr const char* BGM_LEVEL_1       = "bgm_level_1";  // Grassland
    inline constexpr const char* BGM_LEVEL_2       = "bgm_level_2";  // Underground
    inline constexpr const char* BGM_LEVEL_3       = "bgm_level_3";  // Castle
}

// Manages all game audio: sound effects (Sound) and background music (Music).
// All methods are static — no instance needed.
// Call AudioManager::Init() once after InitAudioDevice(), and
// AudioManager::Shutdown() before CloseAudioDevice().
class AudioManager {
public:
    // -----------------------------------------------------------------------
    // Lifecycle
    // -----------------------------------------------------------------------

    // Loads every audio file from assets/audio into the manager.
    // Call once during game initialisation (after Init()).
    static void LoadAll();

    // Must be called once after InitAudioDevice().
    static void Init();

    // Updates streaming music buffers. Call every frame in the game loop.
    static void Update();

    // Unloads all sounds and music, then resets the manager.
    static void Shutdown();

    // -----------------------------------------------------------------------
    // Sound effects (short, loaded fully into RAM)
    // -----------------------------------------------------------------------

    // Load a sound from file and store under 'key'. No-op if key already exists.
    static void LoadSound(const std::string& key, const std::string& filePath);

    // Play a sound by key. Safe no-op if key not found.
    static void PlaySFX(const std::string& key);

    // Stop a currently playing sound by key.
    static void StopSFX(const std::string& key);

    // Check whether a sound is currently playing.
    static bool IsSFXPlaying(const std::string& key);

    // Set volume [0.0, 1.0] for a specific sound.
    static void SetSFXVolume(const std::string& key, float volume);

    // Unload a single sound and remove it from the map.
    static void UnloadSound(const std::string& key);

    // -----------------------------------------------------------------------
    // Music / BGM (streaming from disk)
    // -----------------------------------------------------------------------

    // Load a music stream from file and store under 'key'. No-op if key exists.
    static void LoadMusic(const std::string& key, const std::string& filePath);

    // Play (or restart) a music track by key.
    // If 'exclusive' is true (default), stops any currently playing track first.
    static void PlayBGM(const std::string& key, bool exclusive = true);

    // Pause the currently playing music track.
    static void PauseBGM();

    // Resume the paused music track.
    static void ResumeBGM();

    // Stop the currently playing music track.
    static void StopBGM();

    // Returns the key of the currently playing BGM, or "" if none.
    static const std::string& CurrentBGM();

    // Set global BGM volume [0.0, 1.0].
    static void SetBGMVolume(float volume);

    // Set global SFX volume multiplier [0.0, 1.0] applied to all sounds.
    static void SetMasterSFXVolume(float volume);

    // Unload a single music stream and remove it from the map.
    static void UnloadMusic(const std::string& key);

    // -----------------------------------------------------------------------
    // Queries
    // -----------------------------------------------------------------------

    static bool HasSound(const std::string& key);
    static bool HasMusic(const std::string& key);

private:
    static std::unordered_map<std::string, Sound>  sounds;
    static std::unordered_map<std::string, Music>  musicTracks;
    static std::string  currentBGMKey;
    static float        bgmVolume;
    static float        masterSFXVolume;
    static bool         initialized;
};
