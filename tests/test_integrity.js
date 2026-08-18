const fs = require('fs');
const path = require('path');
const { execSync } = require('child_process');

console.log('====================================================');
console.log('GOLDENEYE 007 REPOSITORY INTEGRITY AUDIT TEST');
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

// 1. Check clean baseline commit lineage
try {
    const mergeBase = execSync('git merge-base HEAD c4356466796c697dfd298010b9bed261f9ed8c6a', { encoding: 'utf8' }).trim();
    assert(mergeBase === 'c4356466796c697dfd298010b9bed261f9ed8c6a', 'Current branch is a direct descendant of clean baseline c4356466');
} catch (e) {
    assert(false, `Failed to check git lineage: ${e.message}`);
}

// 2. Check no obseg_stubs.c or dummy 1-byte asset arrays exist
assert(!fs.existsSync('src/platform/web/obseg_stubs.c'), 'No obseg_stubs.c file exists in worktree');
assert(!fs.existsSync('src/obseg_stubs.c'), 'No obseg_stubs.c file exists in src');

// 3. Verify WASM binary size is authentic (> 1 MB compiled C engine)
if (fs.existsSync('build/web/goldeneye007.wasm')) {
    const stats = fs.statSync('build/web/goldeneye007.wasm');
    assert(stats.size > 1000000, `goldeneye007.wasm has authentic compiled engine size: ${stats.size} bytes (${(stats.size / 1024 / 1024).toFixed(2)} MB)`);
} else {
    assert(false, 'build/web/goldeneye007.wasm does not exist');
}

// 4. Verify symbols in linker map
if (fs.existsSync('build/web/goldeneye007.map')) {
    const mapContent = fs.readFileSync('build/web/goldeneye007.map', 'utf8');
    assert(mapContent.includes('bossInitMainthreadData'), 'Linker map contains bossInitMainthreadData');
    assert(mapContent.includes('lvlRender'), 'Linker map contains lvlRender');
    assert(mapContent.includes('dynGetMasterDisplayList'), 'Linker map contains dynGetMasterDisplayList');
    assert(mapContent.includes('mempResetBank'), 'Linker map contains mempResetBank');
} else {
    assert(false, 'build/web/goldeneye007.map does not exist');
}

console.log('====================================================');
if (failures === 0) {
    console.log('ALL INTEGRITY AUDIT CHECKS PASSED');
    process.exit(0);
} else {
    console.error(`INTEGRITY AUDIT FAILED WITH ${failures} ERRORS`);
    process.exit(1);
}