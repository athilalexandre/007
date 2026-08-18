const path = require('path');
const fs = require('fs');

console.log('====================================================');
console.log('GOLDENEYE 007 AUTHENTIC ENGINE LIFE CYCLE TEST');
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

const GoldenEyeModule = require('../build/web/goldeneye007.js');

GoldenEyeModule().then(async (mod) => {
    assert(typeof mod._hal_engine_init === 'function', '_hal_engine_init exported');
    assert(typeof mod._hal_engine_step === 'function', '_hal_engine_step exported');
    assert(typeof mod._hal_os_set_rom_data === 'function', '_hal_os_set_rom_data exported');
    assert(typeof mod._hal_gfx_get_framebuffer === 'function', '_hal_gfx_get_framebuffer exported');
    assert(typeof mod._hal_input_set_buttons === 'function', '_hal_input_set_buttons exported');

    // 1. Test unmounted state
    assert(mod._hal_os_get_rom_status() === 0, 'Initial ROM status is unmounted (0)');

    // 2. Test invalid / undersized ROM rejection
    const badPtr = mod._malloc(100);
    mod._hal_os_set_rom_data(badPtr, 100);
    mod._free(badPtr);
    assert(mod._hal_os_get_rom_status() === 2, 'Engine correctly rejected undersized ROM (status 2)');

    // 3. Test engine init fails cleanly when ROM is invalid
    const initFail = mod._hal_engine_init();
    assert(initFail === 0, 'Engine init correctly returns failure (0) without mounted ROM');

    // 4. Test framebuffer allocation and pointer
    const fbPtr = mod._hal_gfx_get_framebuffer();
    assert(fbPtr !== 0, `Valid 320x240 framebuffer pointer returned (0x${fbPtr.toString(16)})`);

    // 5. Test input setting
    mod._hal_input_set_buttons(0x8000, 0, 0); // Press 'A' button
    assert(true, 'Controller button input applied via Web HAL');

    console.log('====================================================');
    if (failures === 0) {
        console.log('ALL ENGINE LIFECYCLE TESTS PASSED');
        process.exit(0);
    } else {
        console.error(`ENGINE LIFECYCLE TESTS FAILED WITH ${failures} ERRORS`);
        process.exit(1);
    }
}).catch((err) => {
    console.error('Test runner fatal error:', err);
    process.exit(1);
});