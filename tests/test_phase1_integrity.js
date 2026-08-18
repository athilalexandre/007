const fs = require('fs');
const path = require('path');
const crypto = require('crypto');
const { execSync } = require('child_process');

console.log("================================================================================");
console.log("   GoldenEye 007 WebAssembly Port - Phase 1 Corrective Integrity Test Suite     ");
console.log("================================================================================\n");

let passed = 0;
let failed = 0;

function check(name, condition, details = "") {
    if (condition) {
        passed++;
        console.log(`[PASS] ${name}`);
        if (details) console.log(`       ${details}`);
    } else {
        failed++;
        console.error(`[FAIL] ${name}`);
        if (details) console.error(`       ${details}`);
    }
}

// 1. Check aicommands.def byte-for-byte hash against clean upstream baseline
const aiDefPath = path.join(__dirname, '../src/aicommands.def');
const aiDefContent = fs.readFileSync(aiDefPath);
const aiDefSha256 = crypto.createHash('sha256').update(aiDefContent).digest('hex').toUpperCase();
const EXPECTED_AI_SHA256 = '3AE0280014B6748C922E4C2F2AB0A6C1122F96192F473C537632C584BA252A9C';
check("src/aicommands.def matches clean baseline byte-for-byte", aiDefSha256 === EXPECTED_AI_SHA256, `SHA-256: ${aiDefSha256}`);

// 2. Check c_data_filler.c is restored to clean baseline
const cDataPath = path.join(__dirname, '../src/c_data_filler.c');
const cDataContent = fs.readFileSync(cDataPath, 'utf-8').trim();
check("src/c_data_filler.c is clean baseline (no fake ROM segment stubs)", !cDataContent.includes('obseg_'), `Length: ${cDataContent.length} chars`);

// 3. Check obseg_stubs.c does not exist
const obsegStubsPath = path.join(__dirname, '../src/platform/web/obseg_stubs.c');
check("src/platform/web/obseg_stubs.c is removed", !fs.existsSync(obsegStubsPath));

// 4. Check git branch is feat/engine-proof-of-life-v2
let currentBranch = "";
try {
    currentBranch = execSync('git rev-parse --abbrev-ref HEAD', { cwd: path.join(__dirname, '..') }).toString().trim();
} catch (e) {}
check("Active Git branch is feat/engine-proof-of-life-v2", currentBranch === 'feat/engine-proof-of-life-v2', `Branch: ${currentBranch}`);

// 5. Check clean baseline commit in history
let mergeBase = "";
try {
    mergeBase = execSync('git merge-base HEAD c4356466796c697dfd298010b9bed261f9ed8c6a', { cwd: path.join(__dirname, '..') }).toString().trim();
} catch (e) {}
check("Branch ancestor is clean baseline c4356466796c697dfd298010b9bed261f9ed8c6a", mergeBase === 'c4356466796c697dfd298010b9bed261f9ed8c6a', `Merge-base: ${mergeBase}`);

// 6. Check generated WebAssembly binary and map file
const wasmPath = path.join(__dirname, '../build/web/goldeneye007.wasm');
const jsPath = path.join(__dirname, '../build/web/goldeneye007.js');
const mapPath = path.join(__dirname, '../build/web/goldeneye007.map');

check("build/web/goldeneye007.wasm exists", fs.existsSync(wasmPath), fs.existsSync(wasmPath) ? `Size: ${fs.statSync(wasmPath).size} bytes` : "");
check("build/web/goldeneye007.js exists", fs.existsSync(jsPath), fs.existsSync(jsPath) ? `Size: ${fs.statSync(jsPath).size} bytes` : "");
check("build/web/goldeneye007.map exists", fs.existsSync(mapPath), fs.existsSync(mapPath) ? `Size: ${fs.statSync(mapPath).size} bytes` : "");

// 7. Check Linker Map for real original engine functions
if (fs.existsSync(mapPath)) {
    const mapContent = fs.readFileSync(mapPath, 'utf-8');
    const requiredSymbols = [
        'bossInitMainthreadData',
        'mempInit',
        'memaInit',
        'debInit',
        'joyInit',
        'viInit',
        'obInit',
        'rspInit',
        'dynInit',
        'stanInit',
        'gameInit',
        'romCopy',
        'langInit'
    ];
    for (const sym of requiredSymbols) {
        check(`Linker map includes authentic engine symbol: ${sym}`, mapContent.includes(sym));
    }
}

// 8. Test WASM Execution in runtime
async function testExecution() {
    const ModuleFactory = require('../build/web/goldeneye007.js');
    const Module = await ModuleFactory();
    
    check("Exported hal_engine_init function is available", typeof Module._hal_engine_init === 'function');
    check("Exported hal_engine_is_initialized function is available", typeof Module._hal_engine_is_initialized === 'function');
    check("Exported hal_os_set_rom_data function is available", typeof Module._hal_os_set_rom_data === 'function');
    check("Exported hal_os_get_rom_status function is available", typeof Module._hal_os_get_rom_status === 'function');

    const initResult = Module._hal_engine_init();
    check("hal_engine_init() returns 1 (success)", initResult === 1);
    check("hal_engine_is_initialized() returns 1 (success)", Module._hal_engine_is_initialized() === 1);

    console.log("\n================================================================================");
    console.log(`Results: ${passed} passed, ${failed} failed.`);
    console.log("================================================================================");

    if (failed > 0) {
        process.exit(1);
    }
}

testExecution().catch(err => {
    console.error("Runtime test failure:", err);
    process.exit(1);
});