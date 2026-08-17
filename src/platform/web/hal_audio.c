/**
 * GoldenEye 007 Web Port - Audio HAL (Stub)
 * Replaces N64 libaudio with Web Audio API backend.
 * This is a STUB - will be implemented incrementally.
 */
#ifdef TARGET_WEB

#include <stdio.h>
#include <string.h>

/* All libaudio functions are stubbed out for now.
 * The game will run silently until Phase 3 audio implementation.
 *
 * Key N64 audio systems to reimplement:
 * - VADPCM sample decoding (compressed audio)
 * - MIDI/sequence playback (music tracks)
 * - Sound effect mixing (gunshots, footsteps, etc.)
 * - 3D spatial audio (positional sound)
 */

void hal_audio_init(void) {
    printf("[HAL_AUDIO] Audio system initialized (stub - no sound yet)\n");
}

void hal_audio_update(void) {
    /* Called once per frame to process audio buffer */
}

void hal_audio_play_sfx(int sfx_id, float x, float y, float z) {
    /* TODO: Play sound effect with 3D positioning via Web Audio API */
}

void hal_audio_play_music(int music_id) {
    /* TODO: Start MIDI/sequence playback */
}

void hal_audio_stop_music(void) {
    /* TODO: Stop current music track */
}

#endif /* TARGET_WEB */