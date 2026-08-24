const path = require('path');
const fs = require('fs');
const crypto = require('crypto');

console.log('====================================================');
console.log('GOLDENEYE 007 AUTHENTIC ENGINE E2E INTEGRATION TEST');
console.log('====================================================');

let failures = 0;
function assert(condition, message) {
    if (!condition) {
        console.error(`[FAIL] ${message}`);
        failures++;
    } else {
        console.log(`[PASS] ${message}`);
    }
}

const ROM_PATH = path.join(__dirname, '..', 'assets', 'ramrom', 'GoldenEye 007 (USA).z64');
const EXPECTED_SHA1 = 'abe01e4aeb033b6c0836819f549c791b26cfde83';

if (!fs.existsSync(ROM_PATH)) {
    console.error(`[FATAL] Local ROM not found at ${ROM_PATH}`);
    process.exit(1);
}

const romBuffer = fs.readFileSync(ROM_PATH);
const sha1 = crypto.createHash('sha1').update(romBuffer).digest('hex');

assert(romBuffer.length === 12582912, `ROM size is exact 12,582,912 bytes (${romBuffer.length})`);
assert(sha1 === EXPECTED_SHA1, `ROM SHA-1 is authentic (${sha1})`);

// Test format normalizations
const v64 = Buffer.alloc(romBuffer.length);
for (let i = 0; i < romBuffer.length; i += 2) {
    v64[i] = romBuffer[i + 1];
    v64[i + 1] = romBuffer[i];
}
const n64 = Buffer.alloc(romBuffer.length);
for (let i = 0; i < romBuffer.length; i += 4) {
    n64[i] = romBuffer[i + 3];
    n64[i + 1] = romBuffer[i + 2];
    n64[i + 2] = romBuffer[i + 1];
    n64[i + 3] = romBuffer[i];
}

function normalize(buf) {
    const magic = buf.readUInt32BE(0);
    if (magic === 0x80371240) return buf;
    if (magic === 0x37804012) {
        const out = Buffer.alloc(buf.length);
        for (let i = 0; i < buf.length; i += 2) {
            out[i] = buf[i + 1];
            out[i + 1] = buf[i];
        }
        return out;
    }
    if (magic === 0x40123780) {
        const out = Buffer.alloc(buf.length);
        for (let i = 0; i < buf.length; i += 4) {
            out[i] = buf[i + 3];
            out[i + 1] = buf[i + 2];
            out[i + 2] = buf[i + 1];
            out[i + 3] = buf[i];
        }
        return out;
    }
    return null;
}

assert(crypto.createHash('sha1').update(normalize(v64)).digest('hex') === EXPECTED_SHA1, '.v64 format correctly normalizes to canonical SHA-1');
assert(crypto.createHash('sha1').update(normalize(n64)).digest('hex') === EXPECTED_SHA1, '.n64 format correctly normalizes to canonical SHA-1');

const GoldenEyeModule = require('../build/web/goldeneye007.js');

GoldenEyeModule().then(async (mod) => {
    assert(typeof mod._hal_engine_init === 'function', '_hal_engine_init exported');
    assert(typeof mod._hal_engine_step === 'function', '_hal_engine_step exported');
    assert(typeof mod._hal_os_set_rom_data === 'function', '_hal_os_set_rom_data exported');
    assert(typeof mod._hal_gfx_get_framebuffer === 'function', '_hal_gfx_get_framebuffer exported');

    // 1. Test invalid / undersized ROM rejection
    const badPtr = mod._malloc(100);
    mod._hal_os_set_rom_data(badPtr, 100);
    mod._free(badPtr);
    assert(mod._hal_os_get_rom_status() === 2, 'Engine correctly rejects undersized input (status 2)');
    assert(mod._hal_engine_init() === -1, 'Engine init fails cleanly when no valid ROM is mounted');

    // 2. Ingest authentic ROM into WASM memory
    const romPtr = mod._malloc(romBuffer.length);
    mod.HEAPU8.set(romBuffer, romPtr);
    mod._hal_os_set_rom_data(romPtr, romBuffer.length);
    mod._free(romPtr);

    assert(mod._hal_os_get_rom_status() === 1, 'Engine successfully validated and mounted authentic ROM');

    // 3. Initialize authentic engine
    console.log('[Test] Calling _hal_engine_init()...');
    const initRes = mod._hal_engine_init();
    assert(initRes === 0, `Engine initialization returned 0 (SUCCESS), got ${initRes}`);
    assert(mod._hal_engine_is_initialized() === 1, '_hal_engine_is_initialized() returns 1');
    assert(mod._hal_engine_get_stage_num() === 90, 'Current stage is LEVELID_TITLE (90 / 0x5A)');

    // 4. Step engine frame loop for 10 frames
    console.log('[Test] Stepping engine frame loop for 10 ticks...');
    for (let f = 0; f < 10; f++) {
        mod._hal_engine_step();
    }

    const ticks = mod._hal_engine_get_frame_count();
    assert(ticks === 10, `Engine completed 10 authentic ticks (ticks=${ticks})`);

    const dmaCount = mod._hal_os_get_dma_transfers();
    const dmaBytes = mod._hal_os_get_dma_bytes();
    assert(dmaCount > 0, `Authentic DMA transfers occurred (transfers=${dmaCount})`);
    assert(dmaBytes > 0, `Authentic DMA bytes read from ROM (bytes=${dmaBytes.toLocaleString()})`);

    // 5. Inspect Framebuffer
    const fbPtr = mod._hal_gfx_get_framebuffer();
    assert(fbPtr !== 0, `Framebuffer allocated at 0x${fbPtr.toString(16)}`);
    const fb16 = new Uint16Array(mod.HEAPU8.buffer, fbPtr, 320 * 240);

    // Transition to Dam or step enough frames for rendering
    if (typeof mod._bossSetLoadedStage === 'function') {
        mod._bossSetLoadedStage(1);
        for (let f = 0; f < 20; f++) {
            mod._hal_engine_step();
        }
    }

    let nonZeroPixels = 0;
    let distinctPixels = new Set();
    for (let i = 0; i < 320 * 240; i++) {
        if (fb16[i] !== 0) nonZeroPixels++;
        distinctPixels.add(fb16[i]);
    }

    console.log(`[Test] Framebuffer non-zero pixels: ${nonZeroPixels} / 76800 (${distinctPixels.size} distinct colors)`);
    assert(distinctPixels.size > 1, 'Framebuffer contains non-uniform authentic rendered graphics');

    console.log('====================================================');
    if (failures === 0) {
        console.log('ALL E2E INTEGRATION TESTS PASSED');
        process.exit(0);
    } else {
        console.error(`E2E INTEGRATION TESTS FAILED WITH ${failures} ERRORS`);
        process.exit(1);
    }
}).catch((err) => {
    console.error('Test runner fatal error:', err);
    process.exit(1);
});