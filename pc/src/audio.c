/**
 * Hotel Dusk: Room 215 (USA) - PC Port Audio Subsystem Implementation
 */

#include "audio.h"
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ACTIVE_VOICES 16

typedef struct {
    uint8_t *data;
    uint32_t length;
    bool loaded;
} SoundSample;

typedef struct {
    uint8_t *pos;
    uint32_t remaining;
    bool active;
    float volume;
} ActiveVoice;

static SoundSample  s_Samples[SFX_COUNT];
static ActiveVoice  s_Voices[MAX_ACTIVE_VOICES];
static SDL_AudioDeviceID s_AudioDevice = 0;
static SDL_AudioSpec s_DeviceSpec;
static bool s_Muted = false;
static bool s_BGMActive = false;
static uint32_t s_BGMPos = 0;

static const char *s_SFXFileNames[SFX_COUNT] = {
    "sfx_text_click.wav",
    "sfx_ok.wav",
    "sfx_cancel.wav",
    "sfx_notebook_open.wav",
    "sfx_page_turn.wav",
    "sfx_notebook_close.wav",
    "sfx_dialogue_next.wav",
    "sfx_pen.wav",
    "sfx_bell.wav",
    "sfx_door.wav",
    "jingle_success.wav",
    "jingle_mystery.wav",
    "jingle_puzzle_enter.wav",
    "jingle_event.wav",
    "jingle_flashback.wav",
    "jingle_gameover.wav"
};

static void AudioCallback(void *userdata, Uint8 *stream, int len) {
    (void)userdata;
    memset(stream, 0, len);
    if (s_Muted) return;

    // 1. Mix background music (looping jingle_mystery / jingle_flashback)
    if (s_BGMActive && s_Samples[SFX_JINGLE_MYSTERY].loaded) {
        SoundSample *bgm = &s_Samples[SFX_JINGLE_MYSTERY];
        int bytes_to_mix = len;
        Uint8 *stream_ptr = stream;

        while (bytes_to_mix > 0) {
            uint32_t available = bgm->length - s_BGMPos;
            uint32_t chunk = (bytes_to_mix < (int)available) ? (uint32_t)bytes_to_mix : available;

            SDL_MixAudioFormat(stream_ptr, bgm->data + s_BGMPos, s_DeviceSpec.format, chunk, SDL_MIX_MAXVOLUME / 3);

            s_BGMPos += chunk;
            if (s_BGMPos >= bgm->length) {
                s_BGMPos = 0; // Loop
            }
            bytes_to_mix -= chunk;
            stream_ptr += chunk;
        }
    }

    // 2. Mix active sound effect voices
    for (int i = 0; i < MAX_ACTIVE_VOICES; i++) {
        if (!s_Voices[i].active) continue;

        uint32_t to_write = (len < (int)s_Voices[i].remaining) ? (uint32_t)len : s_Voices[i].remaining;
        int vol = (int)(SDL_MIX_MAXVOLUME * s_Voices[i].volume);
        if (vol > SDL_MIX_MAXVOLUME) vol = SDL_MIX_MAXVOLUME;

        SDL_MixAudioFormat(stream, s_Voices[i].pos, s_DeviceSpec.format, to_write, vol);

        s_Voices[i].pos += to_write;
        s_Voices[i].remaining -= to_write;

        if (s_Voices[i].remaining == 0) {
            s_Voices[i].active = false;
        }
    }
}

static bool LoadAndConvertSample(const char *filepath, SoundSample *out_sample) {
    SDL_AudioSpec wav_spec;
    Uint8 *wav_buffer = NULL;
    Uint32 wav_length = 0;

    if (!SDL_LoadWAV(filepath, &wav_spec, &wav_buffer, &wav_length)) {
        return false;
    }

    if (wav_spec.format == s_DeviceSpec.format &&
        wav_spec.channels == s_DeviceSpec.channels &&
        wav_spec.freq == s_DeviceSpec.freq) {
        out_sample->data = (uint8_t *)malloc(wav_length);
        if (!out_sample->data) {
            SDL_FreeWAV(wav_buffer);
            return false;
        }
        memcpy(out_sample->data, wav_buffer, wav_length);
        out_sample->length = wav_length;
        out_sample->loaded = true;
        SDL_FreeWAV(wav_buffer);
        return true;
    }

    // Build audio converter to hardware format
    SDL_AudioCVT cvt;
    if (SDL_BuildAudioCVT(&cvt, wav_spec.format, wav_spec.channels, wav_spec.freq,
                          s_DeviceSpec.format, s_DeviceSpec.channels, s_DeviceSpec.freq) < 0) {
        SDL_FreeWAV(wav_buffer);
        return false;
    }

    cvt.len = wav_length;
    cvt.buf = (Uint8 *)malloc(wav_length * cvt.len_mult);
    if (!cvt.buf) {
        SDL_FreeWAV(wav_buffer);
        return false;
    }

    memcpy(cvt.buf, wav_buffer, wav_length);
    SDL_FreeWAV(wav_buffer);

    if (SDL_ConvertAudio(&cvt) < 0) {
        free(cvt.buf);
        return false;
    }

    out_sample->data = (uint8_t *)malloc(cvt.len_cvt);
    if (!out_sample->data) {
        free(cvt.buf);
        return false;
    }
    memcpy(out_sample->data, cvt.buf, cvt.len_cvt);
    out_sample->length = cvt.len_cvt;
    out_sample->loaded = true;
    free(cvt.buf);

    return true;
}

bool Audio_Init(const char *asset_dir) {
    memset(s_Samples, 0, sizeof(s_Samples));
    memset(s_Voices, 0, sizeof(s_Voices));

    SDL_AudioSpec want;
    SDL_zero(want);
    want.freq = 44100;
    want.format = AUDIO_S16SYS;
    want.channels = 2;
    want.samples = 1024;
    want.callback = AudioCallback;

    s_AudioDevice = SDL_OpenAudioDevice(NULL, 0, &want, &s_DeviceSpec, 0);
    if (s_AudioDevice == 0) {
        printf("[WARN] Could not open audio device: %s\n", SDL_GetError());
        return false;
    }

    char sfx_dir[260];
    if (asset_dir && strlen(asset_dir) > 0) {
        if (strstr(asset_dir, "sfx")) {
            strncpy(sfx_dir, asset_dir, sizeof(sfx_dir) - 1);
            sfx_dir[sizeof(sfx_dir) - 1] = '\0';
        } else {
            snprintf(sfx_dir, sizeof(sfx_dir), "%s/sfx", asset_dir);
        }
    } else {
        strcpy(sfx_dir, "assets/sfx");
    }

    int loaded_count = 0;
    for (int i = 0; i < SFX_COUNT; i++) {
        char fullpath[260];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", sfx_dir, s_SFXFileNames[i]);
        if (LoadAndConvertSample(fullpath, &s_Samples[i])) {
            loaded_count++;
        }
    }

    printf("[PC AUDIO] Initialized %d/%d SFX samples (Stereo 44.1kHz)\n", loaded_count, SFX_COUNT);

    // Start playback
    SDL_PauseAudioDevice(s_AudioDevice, 0);
    s_BGMActive = true;
    return true;
}

void Audio_Shutdown(void) {
    if (s_AudioDevice != 0) {
        SDL_CloseAudioDevice(s_AudioDevice);
        s_AudioDevice = 0;
    }
    for (int i = 0; i < SFX_COUNT; i++) {
        if (s_Samples[i].data) {
            free(s_Samples[i].data);
            s_Samples[i].data = NULL;
        }
    }
}

void Audio_PlaySFX(SFX_ID id) {
    if (id < 0 || id >= SFX_COUNT || !s_Samples[id].loaded || s_AudioDevice == 0 || s_Muted) return;

    SDL_LockAudioDevice(s_AudioDevice);
    for (int i = 0; i < MAX_ACTIVE_VOICES; i++) {
        if (!s_Voices[i].active) {
            s_Voices[i].pos = s_Samples[id].data;
            s_Voices[i].remaining = s_Samples[id].length;
            s_Voices[i].volume = (id == SFX_TEXT_CLICK) ? 0.35f : 0.85f;
            s_Voices[i].active = true;
            break;
        }
    }
    SDL_UnlockAudioDevice(s_AudioDevice);
}

void Audio_StartBGM(void) {
    s_BGMActive = true;
}

void Audio_StopBGM(void) {
    s_BGMActive = false;
}

void Audio_ToggleMute(void) {
    s_Muted = !s_Muted;
    printf("[AUDIO] Audio %s\n", s_Muted ? "MUTED" : "UNMUTED");
}

bool Audio_IsMuted(void) {
    return s_Muted;
}
