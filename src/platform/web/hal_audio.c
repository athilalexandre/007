#include <ultra64.h>
#include <PR/libaudio.h>
#include "hal_audio.h"

s32 alCSPGetState(ALCSPlayer *seqp) { return AL_STOPPED; }
void alCSPStop(ALCSPlayer *seqp) {}
void alCSeqNew(ALCSeq *seq, u8 *ptr) {}
void alCSPSetSeq(ALCSPlayer *seqp, ALCSeq *seq) {}
void alCSPSetVol(ALCSPlayer *seqp, s16 vol) {}
void alCSPPlay(ALCSPlayer *seqp) {}
void alEvtqPostEvent(ALEventQueue *evtq, ALEvent *evt, ALMicroTime delta) {}
void alUnlink(ALLink *ln) {}
f32 alCents2Ratio(s32 cents) { return 1.0f; }
void hal_audio_init(void) {}
