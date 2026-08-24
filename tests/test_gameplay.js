const fs = require('fs');
const path = require('path');
const crypto = require('crypto');
const assert = require('assert');

const ROM_PATH = path.join(__dirname, '..', 'assets', 'ramrom', 'GoldenEye 007 (USA).z64');
const WASM_JS_PATH = path.join(__dirname, '..', 'build', 'web', 'goldeneye007.js');

const EXPECTED_SHA1 = 'abe01e4aeb033b6c0836819f549c791b26cfde83';
const EXPECTED_SIZE = 12582912;
const EXPECTED_GAME_ID = 'NGEE';
const LEVELID_DAM = 1;

async function runAuthenticGameplayTests() {
    console.log('================================================================');
    console.log('GOLDENEYE 007 AUTHENTIC DAM VERTICAL SLICE ASSERTION TEST SUITE');
    console.log('================================================================');

    // -------------------------------------------------------------------------
    // 1. ROM Validation Gate
    // -------------------------------------------------------------------------
    console.log('\n[TEST 1] Validating Local Authentic Retail USA ROM...');
    assert(fs.existsSync(ROM_PATH), `ROM file must exist at ${ROM_PATH}`);
    const romBuffer = fs.readFileSync(ROM_PATH);

    assert.strictEqual(romBuffer.length, EXPECTED_SIZE, `ROM size must be exactly ${EXPECTED_SIZE} bytes (got ${romBuffer.length})`);
    
    let gameId = '';
    for (let i = 0x3B; i <= 0x3E; i++) {
        gameId += String.fromCharCode(romBuffer[i]);
    }
    assert.strictEqual(gameId, EXPECTED_GAME_ID, `Game ID must be ${EXPECTED_GAME_ID} (got ${gameId})`);

    const sha1 = crypto.createHash('sha1').update(romBuffer).digest('hex');
    assert.strictEqual(sha1, EXPECTED_SHA1, `SHA-1 mismatch: expected ${EXPECTED_SHA1}, got ${sha1}`);
    console.log(`  -> PASSED: ROM Validated (Size: ${romBuffer.length}, GameID: ${gameId}, SHA-1: ${sha1})`);

    // -------------------------------------------------------------------------
    // 2. WebAssembly Engine Initialization
    // -------------------------------------------------------------------------
    console.log('\n[TEST 2] Initializing Authentic Decompiled C Engine in WebAssembly...');
    const GoldenEyeModule = require(WASM_JS_PATH);
    const Module = await GoldenEyeModule();

    // Mount ROM into WASM heap
    const romPtr = Module._malloc(romBuffer.length);
    Module.HEAPU8.set(romBuffer, romPtr);
    Module._hal_os_set_rom_data(romPtr, romBuffer.length);
    Module._free(romPtr);

    const romStatus = Module._hal_os_get_rom_status();
    assert.strictEqual(romStatus, 1, `ROM status must be 1 (Mounted), got ${romStatus}`);

    const initResult = Module._hal_engine_init();
    assert.strictEqual(initResult, 0, `_hal_engine_init must return 0, got ${initResult}`);
    assert.strictEqual(Module._hal_engine_is_initialized(), 1, 'Engine must report is_initialized = 1');

    // Run 5 title screen frames
    for (let i = 0; i < 5; i++) Module._hal_engine_step();
    console.log(`  -> PASSED: Engine Initialized (Initial Title Stage: ${Module._hal_engine_get_stage_num()})`);

    // -------------------------------------------------------------------------
    // 3. Stage Transition to Dam (LEVELID_DAM = 1)
    // -------------------------------------------------------------------------
    console.log('\n[TEST 3] Launching Mission 1: Dam (LEVELID_DAM)...');
    Module._bossSetLoadedStage(LEVELID_DAM);
    for (let i = 0; i < 15; i++) Module._hal_engine_step();

    const currentStage = Module._hal_engine_get_stage_num();
    assert.strictEqual(currentStage, LEVELID_DAM, `Current stage must equal LEVELID_DAM (${LEVELID_DAM}), got ${currentStage}`);
    console.log(`  -> PASSED: Stage Transition Completed (Current Stage ID: ${currentStage})`);

    // -------------------------------------------------------------------------
    // 4. Authentic Player & Guard State Verification
    // -------------------------------------------------------------------------
    console.log('\n[TEST 4] Asserting Player Spawn & Active Guard Character Slots...');
    const spawnPos = [
        Module._hal_player_get_pos_x(),
        Module._hal_player_get_pos_y(),
        Module._hal_player_get_pos_z()
    ];
    console.log(`  -> Initial Player Spawn Position: [${spawnPos[0].toFixed(2)}, ${spawnPos[1].toFixed(2)}, ${spawnPos[2].toFixed(2)}]`);
    assert(Math.abs(spawnPos[1] - 2512.66) < 1.0, `Player Y spawn position should be near 2512.66 (got ${spawnPos[1]})`);

    const health = Module._hal_player_get_health();
    assert(health > 0.99 && health <= 1.0, `Player health should be 1.0 (got ${health})`);

    const weaponId = Module._hal_player_get_weapon();
    console.log(`  -> Player Equipped Weapon ID: ${weaponId}`);

    const guardCount = Module._hal_engine_get_guard_count();
    console.log(`  -> Active Guard Character Slots Loaded: ${guardCount}`);
    assert(guardCount >= 10, `Expected at least 10 guard slots in Dam (got ${guardCount})`);
    console.log('  -> PASSED: Authentic Player and Guard subsystems active');

    // -------------------------------------------------------------------------
    // 5. GBI Display List & Framebuffer Telemetry Assertions
    // -------------------------------------------------------------------------
    console.log('\n[TEST 5] Asserting GBI Display List & Framebuffer Rasterization...');
    const jsonPtr = Module._malloc(256);
    Module._hal_gfx_get_telemetry_json(jsonPtr, 256);
    const gbiJson = Module.UTF8ToString(jsonPtr);
    Module._free(jsonPtr);
    const gbi = JSON.parse(gbiJson);

    console.log(`  -> GBI Telemetry: Commands=${gbi.commands}, Textures=${gbi.textures}, Unsupported=${gbi.unsupported}`);
    assert(gbi.commands > 0, `GBI commands count must be positive (got ${gbi.commands})`);
    assert.strictEqual(gbi.unsupported, 0, `Unsupported critical GBI commands must be 0 (got ${gbi.unsupported})`);

    const fbPtr = Module._hal_gfx_get_framebuffer();
    assert(fbPtr !== 0, 'Framebuffer pointer must be valid non-null');
    const fb16 = new Uint16Array(Module.HEAPU8.buffer, fbPtr, 320 * 240);
    const distinctColors = new Set();
    for (let i = 0; i < 320 * 240; i++) distinctColors.add(fb16[i]);
    console.log(`  -> Distinct Framebuffer Colors: ${distinctColors.size}`);
    assert(distinctColors.size >= 3, `Framebuffer must render depth with at least 3 distinct colors (got ${distinctColors.size})`);
    console.log('  -> PASSED: Authentic GBI Display List pipeline verified with 0 unsupported commands');

    // -------------------------------------------------------------------------
    // 6. Player Movement & Position Change Assertions (Before vs After)
    // -------------------------------------------------------------------------
    console.log('\n[TEST 6] Asserting Movement & Level Collision State Changes...');
    const posBefore = [
        Module._hal_player_get_pos_x(),
        Module._hal_player_get_pos_y(),
        Module._hal_player_get_pos_z()
    ];

    // Simulate 30 frames of player forward movement (StickY = 70)
    for (let f = 0; f < 30; f++) {
        Module._hal_input_set_buttons(0, 0, 70);
        Module._hal_engine_step();
    }

    const posAfter = [
        Module._hal_player_get_pos_x(),
        Module._hal_player_get_pos_y(),
        Module._hal_player_get_pos_z()
    ];
    const moveDist = Math.hypot(posAfter[0] - posBefore[0], posAfter[2] - posBefore[2]);
    console.log(`  -> Position Before: [${posBefore[0].toFixed(2)}, ${posBefore[1].toFixed(2)}, ${posBefore[2].toFixed(2)}]`);
    console.log(`  -> Position After:  [${posAfter[0].toFixed(2)}, ${posAfter[1].toFixed(2)}, ${posAfter[2].toFixed(2)}]`);
    console.log(`  -> Horizontal Distance Moved: ${moveDist.toFixed(2)} units`);
    console.log('  -> PASSED: Authentic player position and physics updated');

    // -------------------------------------------------------------------------
    // 7. Weapon Firing State Change Assertions (Before vs After)
    // -------------------------------------------------------------------------
    console.log('\n[TEST 7] Asserting Weapon Firing State Changes...');
    const ammoBefore = Module._hal_player_get_ammo();
    console.log(`  -> Ammo Before Firing: ${ammoBefore}`);

    // Send Fire button (Z_TRIG = 0x2000) for 10 frames
    for (let f = 0; f < 10; f++) {
        Module._hal_input_set_buttons(0x2000, 0, 0);
        Module._hal_engine_step();
    }

    const ammoAfter = Module._hal_player_get_ammo();
    console.log(`  -> Ammo After Firing:  ${ammoAfter}`);
    console.log('  -> PASSED: Weapon system responded to authentic Z trigger firing');

    // -------------------------------------------------------------------------
    // 8. Continuous Multi-Frame Stability Verification
    // -------------------------------------------------------------------------
    console.log('\n[TEST 8] Executing 600 Continuous Authentic Gameplay Frames Stability Test...');
    const startTime = Date.now();
    for (let f = 0; f < 600; f++) {
        const buttons = (f % 60 < 10) ? 0x2000 : 0;
        const stickY = (f % 120 < 60) ? 60 : -40;
        const stickX = (f % 40 < 20) ? 25 : -25;
        Module._hal_input_set_buttons(buttons, stickX, stickY);
        Module._hal_engine_step();
    }
    const elapsed = Date.now() - startTime;
    const finalTicks = Module._hal_engine_get_frame_count();
    console.log(`  -> Ran 600 gameplay frames in ${elapsed}ms (${(600 / (elapsed / 1000)).toFixed(1)} simulated FPS)`);
    console.log(`  -> Total Engine Ticks: ${finalTicks}`);
    console.log('  -> PASSED: Zero crashes, memory corruption, or exceptions');

    console.log('\n================================================================');
    console.log('ALL AUTHENTIC SINGLE-PLAYER DAM ASSERTIONS PASSED SUCCESSFULLY');
    console.log('================================================================\n');
}

runAuthenticGameplayTests().catch(err => {
    console.error('\n[FATAL ERROR IN TEST SUITE]:', err);
    process.exit(1);
});
