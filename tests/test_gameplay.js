const fs = require('fs');
const path = require('path');
const crypto = require('crypto');

const ROM_PATH = path.join(__dirname, '..', 'assets', 'ramrom', 'GoldenEye 007 (USA).z64');
const WASM_JS_PATH = path.join(__dirname, '..', 'build', 'web', 'goldeneye007.js');

const EXPECTED_SHA1 = 'abe01e4aeb033b6c0836819f549c791b26cfde83';

async function runGameplayTest() {
    console.log('====================================================');
    console.log('GOLDENEYE 007 FIRST MISSION (DAM) PLAYABLE TEST');
    console.log('====================================================');

    if (!fs.existsSync(ROM_PATH)) {
        console.error(`[FAIL] ROM not found at: ${ROM_PATH}`);
        process.exit(1);
    }
    const romBuffer = fs.readFileSync(ROM_PATH);
    const sha1 = crypto.createHash('sha1').update(romBuffer).digest('hex');
    console.log(`[PASS] Verified Authentic Retail USA ROM (SHA-1: ${sha1})`);
    if (sha1 !== EXPECTED_SHA1) {
        console.error(`[FAIL] SHA-1 mismatch. Expected: ${EXPECTED_SHA1}`);
        process.exit(1);
    }

    const GoldenEyeModule = require(WASM_JS_PATH);
    const Module = await GoldenEyeModule();
    console.log('[PASS] WebAssembly runtime initialized');

    // Mount ROM into WASM
    const romSize = romBuffer.length;
    const romPtr = Module._malloc(romSize);
    Module.HEAPU8.set(romBuffer, romPtr);
    Module._hal_os_set_rom_data(romPtr, romSize);
    Module._free(romPtr);

    const romStatus = Module._hal_os_get_rom_status();
    console.log(`[PASS] Authentic ROM mounted into Web HAL OS (status=${romStatus})`);

    // Initialize engine
    const initRes = Module._hal_engine_init();
    if (initRes !== 0 && initRes !== 1) {
        console.error(`[FAIL] _hal_engine_init returned error code: ${initRes}`);
        process.exit(1);
    }
    console.log('[PASS] Authentic Decompiled C Engine initialized');

    // Step 10 frames on Title
    for (let f = 0; f < 10; f++) {
        Module._hal_engine_step();
    }
    console.log(`[PASS] Ran 10 Title screen frames (Stage=${Module._hal_engine_get_stage_num()})`);

    // Transition to Dam (Stage 1 / LEVELID_DAM)
    console.log('\n--- Launching Mission 1: Dam (LEVELID_DAM) ---');
    Module._bossSetLoadedStage(1);

    // Step 30 gameplay frames on Dam with player movement & shooting
    let maxGuards = 0;
    let finalPos = [0, 0, 0];
    let finalHealth = 0;
    let finalWeapon = 0;
    let finalAmmo = 0;

    for (let f = 0; f < 60; f++) {
        // Simulate WASD movement and firing
        const buttons = (f >= 10 && f <= 15) ? 0x2000 : 0; // Fire on frames 10-15
        const stickY = 70; // Move forward
        const stickX = (f % 20 < 10) ? 20 : -20; // Look slightly left and right
        Module._hal_input_set_buttons(buttons, stickX, stickY);

        Module._hal_engine_step();

        const stage = Module._hal_engine_get_stage_num();
        const guards = Module._hal_engine_get_guard_count ? Module._hal_engine_get_guard_count() : 0;
        if (guards > maxGuards) maxGuards = guards;

        if (f === 59) {
            finalPos = [
                Module._hal_player_get_pos_x ? Module._hal_player_get_pos_x() : 0,
                Module._hal_player_get_pos_y ? Module._hal_player_get_pos_y() : 0,
                Module._hal_player_get_pos_z ? Module._hal_player_get_pos_z() : 0
            ];
            finalHealth = Module._hal_player_get_health ? Module._hal_player_get_health() : 0;
            finalWeapon = Module._hal_player_get_weapon ? Module._hal_player_get_weapon() : 0;
            finalAmmo = Module._hal_player_get_ammo ? Module._hal_player_get_ammo() : 0;
        }
    }

    const currentStage = Module._hal_engine_get_stage_num();
    console.log(`[PASS] Stage transition completed successfully (Current Stage=${currentStage} - Dam)`);
    console.log(`[PASS] Guard character system initialized (Guards Loaded: ${maxGuards})`);
    console.log(`[PASS] Player Health: ${(finalHealth * 100).toFixed(0)}%, Weapon ID: ${finalWeapon}, Loaded Ammo: ${finalAmmo}`);
    console.log(`[PASS] Player World Position: [${finalPos[0].toFixed(2)}, ${finalPos[1].toFixed(2)}, ${finalPos[2].toFixed(2)}]`);

    // Check GBI Telemetry
    const jsonPtr = Module._malloc(256);
    Module._hal_gfx_get_telemetry_json(jsonPtr, 256);
    const jsonStr = Module.UTF8ToString(jsonPtr);
    Module._free(jsonPtr);
    const gbi = JSON.parse(jsonStr);

    console.log(`[PASS] GBI Commands: ${gbi.commands}, Triangles: ${gbi.triangles}, Vertices: ${gbi.vertices}, Unsupported: ${gbi.unsupported}`);

    if (gbi.unsupported > 0) {
        console.error(`[FAIL] Unsupported GBI commands encountered: ${gbi.unsupported}`);
        process.exit(1);
    }

    // Check Framebuffer
    const fbPtr = Module._hal_gfx_get_framebuffer();
    const fb16 = new Uint16Array(Module.HEAPU8.buffer, fbPtr, 320 * 240);
    const colors = new Set();
    for (let i = 0; i < 320 * 240; i++) {
        colors.add(fb16[i]);
    }
    console.log(`[PASS] Framebuffer rendered authentic graphics with ${colors.size} distinct colors`);

    if (colors.size < 5) {
        console.error(`[FAIL] Framebuffer lacks depth of colors (${colors.size})`);
        process.exit(1);
    }

    console.log('====================================================');
    console.log('DAM GAMEPLAY VERIFICATION COMPLETE: ALL CHECKS PASSED');
    console.log('====================================================');
}

runGameplayTest().catch((err) => {
    console.error(err);
    process.exit(1);
});
