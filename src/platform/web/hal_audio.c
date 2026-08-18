/**
 * GoldenEye 007 Web Port ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â Audio HAL & libultra audio stubs
 */
#ifdef TARGET_WEB

#include <ultra64.h>
#include <PR/os.h>
#include <PR/libaudio.h>
#include "sched.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

OSSched os_scheduler;

void hal_audio_init(void) {
    printf("[HAL_AUDIO] Audio system initialized (WebAudio bridge ready)\n");
}

void hal_audio_update(void) {
}

void hal_audio_play_sfx(int sfx_id, float x, float y, float z) {
}

void hal_audio_play_music(int music_id) {
}

void hal_audio_stop_music(void) {
}

/* === Libultra Audio / AI Stubs === */

s32 osAiSetFrequency(u32 freq) {
    return freq;
}

s32 osAiSetNextBuffer(void *buf, u32 size) {
    return 0;
}

u32 osAiGetLength(void) {
    return 0;
}

u32 osVirtualToPhysical(void *vaddr) {
    return (u32)(uintptr_t)vaddr;
}

void alInit(ALGlobals *glob, ALSynConfig *c) {
}

void alClose(ALGlobals *glob) {
}

void alLink(ALLink *element, ALLink *after) {
}

void alUnlink(ALLink *element) {
}

void* alHeapDBAlloc(u8 *file, s32 line, ALHeap *hp, s32 num, s32 size) {
    return malloc(num * size);
}

#endif /* TARGET_WEB */
Acmd* alAudioFrame(Acmd *cmdList, s32 *cmdLen, s16 *outBuf, s32 outLen) {
    if (cmdLen) *cmdLen = 0;
    return cmdList;
}
void alHeapInit(ALHeap *hp, u8 *base, s32 len) {
    if (hp) {
        hp->base = base;
        hp->len  = len;
        hp->cur  = base;
        hp->count = 0;
    }
}

void alBnkfNew(ALBankFile *file, u8 *table) {
}

/* ROM Audio Segment Symbols */
u8 _sfxtblSegmentRomStart[4] = {0};
u8 _sfxctlSegmentRomStart[4] = {0};
u8 _instrumentstblSegmentRomStart[4] = {0};
u8 _seqSegmentRomStart[4] = {0};
u8 _seqtblSegmentRomStart[4] = {0};
u8 _instrumentsctlSegmentRomStart[4] = {0};
u8 _musicsampletblSegmentRomStart[4] = {0};

void alCSPNew(ALCSPlayer *seqp, ALSeqpConfig *c) { }
void alCSPSetBank(ALCSPlayer *seqp, ALBank *b) { }
void alSeqpSetBank(ALSeqPlayer *seqp, ALBank *b) { }
s32  alCSPGetState(ALCSPlayer *seqp) { return 0; }
void alCSPStop(ALCSPlayer *seqp) { }
void alCSPPlay(ALCSPlayer *seqp) { }
void alCSPSetSeq(ALCSPlayer *seqp, ALCSeq *seq) { }
void alCSeqNew(ALCSeq *seq, u8 *ptr) { }
void alCSPSetVol(ALCSPlayer *seqp, s16 vol) { }
void alCSPSetPan(ALCSPlayer *seqp, u8 chan, ALPan pan) { }
void alCSPSetPitch(ALCSPlayer *seqp, f32 pitch) { }
void alCSPDelete(ALCSPlayer *seqp) { }
void alSeqpDelete(ALSeqPlayer *seqp) { }
void alSeqpNew(ALSeqPlayer *seqp, ALSeqpConfig *c) { }
void alSeqpPlay(ALSeqPlayer *seqp) { }
void alSeqpStop(ALSeqPlayer *seqp) { }
void alSeqpSetSeq(ALSeqPlayer *seqp, ALSeq *seq) { }
void alSeqpSetVol(ALSeqPlayer *seqp, s16 vol) { }
void alSeqNew(ALSeq *seq, u8 *ptr, s32 len) { }
ALGlobals *alGlobals = NULL;

void alEvtqNew(ALEventQueue *evtq, ALEventListItem *items, s32 itemCount) { }
ALMicroTime alEvtqNextEvent(ALEventQueue *evtq, ALEvent *evt) { return 0; }
void alEvtqPostEvent(ALEventQueue *evtq, ALEvent *evt, ALMicroTime delta) { }
void alSynAddPlayer(ALSynth *drvr, ALPlayer *client) { }
s32  alSynAllocVoice(ALSynth *s, ALVoice *v, ALVoiceConfig *vc) { return 0; }
void alSynStartVoice(ALSynth *s, ALVoice *v, ALWaveTable *w) { }
void alSynStopVoice(ALSynth *s, ALVoice *v) { }
void alSynSetVol(ALSynth *s, ALVoice *v, s16 vol, ALMicroTime t) { }
void alSynSetPitch(ALSynth *s, ALVoice *v, f32 pitch) { }
void alSynSetPan(ALSynth *s, ALVoice *v, ALPan pan) { }
void alSynSetFXMix(ALSynth *s, ALVoice *v, u8 fxmix) { }
void alSynFreeVoice(ALSynth *s, ALVoice *v) { }
f32 alCents2Ratio(s32 cents) {
    return powf(2.0f, (f32)cents / 1200.0f);
}