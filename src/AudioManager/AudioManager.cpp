#include "AudioManager.h"
#include <stdexcept>

// ---------------------------------------------------------------------------
// Static member definitions
// ---------------------------------------------------------------------------
std::unordered_map<std::string, Sound> AudioManager::sounds;
std::unordered_map<std::string, Music> AudioManager::musicTracks;
std::string AudioManager::currentBGMKey;
float       AudioManager::bgmVolume      = 1.0f;
float       AudioManager::masterSFXVolume = 1.0f;
bool        AudioManager::initialized    = false;
bool        AudioManager::starmanActive  = false;

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

void AudioManager::Init() {
    if (initialized) return;
    initialized = true;
    TraceLog(LOG_INFO, "AudioManager: Initialized");
}

void AudioManager::LoadAll() {
    // -----------------------------------------------------------------------
    // SFX — one file per action, loaded from assets/audio/<folder>/<file>
    // -----------------------------------------------------------------------
    LoadSound(AudioKey::JUMP_SMALL,     "assets/audio/jump (small)/jump-small.wav");
    LoadSound(AudioKey::JUMP_SUPER,     "assets/audio/jump (super)/jump-super.wav");
    LoadSound(AudioKey::HIT_COIN,       "assets/audio/hit coin/hit_coin.wav");
    LoadSound(AudioKey::HIT_ENEMY,      "assets/audio/hit enemy/hit_enemy.wav");
    LoadSound(AudioKey::MARIO_DIE,      "assets/audio/mario die/mario_die.wav");
    LoadSound(AudioKey::PIPE_TRAVEL,    "assets/audio/pipe travel/pipe_travel.wav");
    LoadSound(AudioKey::POWER_DOWN,     "assets/audio/power down/power_down.wav");
    LoadSound(AudioKey::POWER_UP,       "assets/audio/power up/powerup.wav");
    LoadSound(AudioKey::POWERUP_APPEARS,"assets/audio/power-ups appear/powerup_appears.wav");
    LoadSound(AudioKey::DOWN_FLAG_POLE, "assets/audio/down the flag pole/down_the_flag_pole.wav");
    LoadSound(AudioKey::FIREBALL,       "assets/audio/Fireball/Fireball.mp3");

    TraceLog(LOG_INFO, "AudioManager: All SFX loaded");

    // -----------------------------------------------------------------------
    // BGM — streaming music, one per level (+ starman invincibility theme)
    // -----------------------------------------------------------------------
    LoadMusic(AudioKey::BGM_LEVEL_1, "assets/audio/back ground music/LV1/lv1.mp3");
    LoadMusic(AudioKey::BGM_LEVEL_2, "assets/audio/back ground music/LV2/lv2.mp3");
    LoadMusic(AudioKey::BGM_LEVEL_3, "assets/audio/back ground music/LV3/lv3.mp3");
    LoadMusic(AudioKey::STARMAN,     "assets/audio/Invincible Star/Invincible Star.mp3");

    TraceLog(LOG_INFO, "AudioManager: All BGM loaded");
}

void AudioManager::Update() {
    if (!initialized) return;
    if (starmanActive) {
        auto it = musicTracks.find(AudioKey::STARMAN);
        if (it != musicTracks.end()) {
            UpdateMusicStream(it->second);
        }
    } else if (!currentBGMKey.empty()) {
        auto it = musicTracks.find(currentBGMKey);
        if (it != musicTracks.end()) {
            UpdateMusicStream(it->second);
        }
    }
}

void AudioManager::Shutdown() {
    StopBGM();

    for (auto& [key, sound] : sounds) {
        ::UnloadSound(sound);
    }
    sounds.clear();

    for (auto& [key, music] : musicTracks) {
        ::UnloadMusicStream(music);
    }
    musicTracks.clear();

    currentBGMKey.clear();
    initialized = false;
    TraceLog(LOG_INFO, "AudioManager: Shutdown — all audio resources freed");
}

// ---------------------------------------------------------------------------
// Sound effects
// ---------------------------------------------------------------------------

void AudioManager::LoadSound(const std::string& key, const std::string& filePath) {
    if (sounds.find(key) != sounds.end()) return;  // already loaded

    Sound s = ::LoadSound(filePath.c_str());
    if (s.stream.buffer == nullptr) {
        TraceLog(LOG_ERROR, "AudioManager: Failed to load sound '%s' from '%s'",
                 key.c_str(), filePath.c_str());
        return;
    }
    SetSoundVolume(s, masterSFXVolume);
    sounds[key] = s;
    TraceLog(LOG_INFO, "AudioManager: Loaded sound '%s'", key.c_str());
}

void AudioManager::PlaySFX(const std::string& key) {
    auto it = sounds.find(key);
    if (it == sounds.end()) {
        TraceLog(LOG_WARNING, "AudioManager: PlaySFX — sound '%s' not found", key.c_str());
        return;
    }
    ::PlaySound(it->second);
}

void AudioManager::StopSFX(const std::string& key) {
    auto it = sounds.find(key);
    if (it != sounds.end()) {
        ::StopSound(it->second);
    }
}

bool AudioManager::IsSFXPlaying(const std::string& key) {
    auto it = sounds.find(key);
    if (it == sounds.end()) return false;
    return ::IsSoundPlaying(it->second);
}

void AudioManager::SetSFXVolume(const std::string& key, float volume) {
    auto it = sounds.find(key);
    if (it != sounds.end()) {
        SetSoundVolume(it->second, volume);
    }
}

void AudioManager::UnloadSound(const std::string& key) {
    auto it = sounds.find(key);
    if (it != sounds.end()) {
        ::UnloadSound(it->second);
        sounds.erase(it);
    }
}

// ---------------------------------------------------------------------------
// Music / BGM
// ---------------------------------------------------------------------------

void AudioManager::LoadMusic(const std::string& key, const std::string& filePath) {
    if (musicTracks.find(key) != musicTracks.end()) return;  // already loaded

    Music m = ::LoadMusicStream(filePath.c_str());
    if (m.stream.buffer == nullptr) {
        TraceLog(LOG_ERROR, "AudioManager: Failed to load music '%s' from '%s'",
                 key.c_str(), filePath.c_str());
        return;
    }
    SetMusicVolume(m, bgmVolume);
    musicTracks[key] = m;
    TraceLog(LOG_INFO, "AudioManager: Loaded music '%s'", key.c_str());
}

void AudioManager::PlayBGM(const std::string& key, bool exclusive) {
    if (starmanActive) {
        starmanActive = false;
        auto itStar = musicTracks.find(AudioKey::STARMAN);
        if (itStar != musicTracks.end()) {
            ::StopMusicStream(itStar->second);
        }
    }

    if (exclusive && !currentBGMKey.empty()) {
        auto it = musicTracks.find(currentBGMKey);
        if (it != musicTracks.end()) {
            ::StopMusicStream(it->second);
        }
    }

    auto it = musicTracks.find(key);
    if (it == musicTracks.end()) {
        TraceLog(LOG_WARNING, "AudioManager: PlayBGM — music '%s' not found", key.c_str());
        return;
    }

    currentBGMKey = key;
    ::PlayMusicStream(it->second);
}

void AudioManager::PauseBGM() {
    if (starmanActive) {
        auto itStar = musicTracks.find(AudioKey::STARMAN);
        if (itStar != musicTracks.end()) {
            ::PauseMusicStream(itStar->second);
        }
    }
    if (!currentBGMKey.empty()) {
        auto it = musicTracks.find(currentBGMKey);
        if (it != musicTracks.end()) {
            ::PauseMusicStream(it->second);
        }
    }
}

void AudioManager::ResumeBGM() {
    if (starmanActive) {
        auto itStar = musicTracks.find(AudioKey::STARMAN);
        if (itStar != musicTracks.end()) {
            ::ResumeMusicStream(itStar->second);
        }
    } else if (!currentBGMKey.empty()) {
        auto it = musicTracks.find(currentBGMKey);
        if (it != musicTracks.end()) {
            ::ResumeMusicStream(it->second);
        }
    }
}

void AudioManager::StopBGM() {
    if (starmanActive) {
        starmanActive = false;
        auto itStar = musicTracks.find(AudioKey::STARMAN);
        if (itStar != musicTracks.end()) {
            ::StopMusicStream(itStar->second);
        }
    }
    if (!currentBGMKey.empty()) {
        auto it = musicTracks.find(currentBGMKey);
        if (it != musicTracks.end()) {
            ::StopMusicStream(it->second);
        }
        currentBGMKey.clear();
    }
}

void AudioManager::StartStarmanBGM() {
    auto itStar = musicTracks.find(AudioKey::STARMAN);
    if (itStar == musicTracks.end()) {
        TraceLog(LOG_WARNING, "AudioManager: Starman music track not found");
        return;
    }

    if (!starmanActive) {
        starmanActive = true;
        // Pause stage BGM if one was playing
        if (!currentBGMKey.empty()) {
            auto itBgm = musicTracks.find(currentBGMKey);
            if (itBgm != musicTracks.end()) {
                ::PauseMusicStream(itBgm->second);
            }
        }
    }

    // (Re)start starman music from beginning
    ::StopMusicStream(itStar->second);
    ::PlayMusicStream(itStar->second);
}

void AudioManager::StopStarmanBGM() {
    if (!starmanActive) return;
    starmanActive = false;

    auto itStar = musicTracks.find(AudioKey::STARMAN);
    if (itStar != musicTracks.end()) {
        ::StopMusicStream(itStar->second);
    }

    // Resume stage BGM if one was paused
    if (!currentBGMKey.empty()) {
        auto itBgm = musicTracks.find(currentBGMKey);
        if (itBgm != musicTracks.end()) {
            ::ResumeMusicStream(itBgm->second);
        }
    }
}

bool AudioManager::IsStarmanBGMPlaying() {
    return starmanActive;
}

const std::string& AudioManager::CurrentBGM() {
    return currentBGMKey;
}

void AudioManager::SetBGMVolume(float volume) {
    bgmVolume = volume;
    for (auto& [key, music] : musicTracks) {
        SetMusicVolume(music, bgmVolume);
    }
}

void AudioManager::SetMasterSFXVolume(float volume) {
    masterSFXVolume = volume;
    for (auto& [key, sound] : sounds) {
        SetSoundVolume(sound, masterSFXVolume);
    }
}

void AudioManager::UnloadMusic(const std::string& key) {
    auto it = musicTracks.find(key);
    if (it != musicTracks.end()) {
        if (currentBGMKey == key) {
            ::StopMusicStream(it->second);
            currentBGMKey.clear();
        }
        if (key == AudioKey::STARMAN) {
            starmanActive = false;
        }
        ::UnloadMusicStream(it->second);
        musicTracks.erase(it);
    }
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

bool AudioManager::HasSound(const std::string& key) {
    return sounds.find(key) != sounds.end();
}

bool AudioManager::HasMusic(const std::string& key) {
    return musicTracks.find(key) != musicTracks.end();
}

