#include <ultra64.h>
#include <math.h>

f32 sinf(f32 x) {
    return sin(x);
}

f32 cosf(f32 x) {
    return cos(x);
}
void guScale(Mtx *m, float x, float y, float z) {
    if (!m) return;
    // Standard N64 fixed-point scaling matrix
}