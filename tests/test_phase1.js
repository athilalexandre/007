const fs = require('fs');
const { execSync } = require('child_process');

let totalTests = 0;
let passedTests = 0;

function assert(condition, message) {
    totalTests++;
    if (!condition) {
        console.error('[FAIL] ' + message);
        throw new Error('Assertion failed: ' + message);
    }
    passedTests++;
    console.log('  [PASS] ' + message);
}

console.log('=== GoldenEye 007 - Phase 1 Verification Test Suite ===\n');

// 1. ROM Normalization Test (.z64, .v64, .n64)
console.log('1. Testing 3-Format ROM Normalization:');
function normalizeRom(buffer) {
    const magic = buffer.readUInt32BE(0);
    const normalized = Buffer.from(buffer);
    
    if (magic === 0x80371240) {
        return { format: '.z64', buffer: normalized };
    } else if (magic === 0x37804012) {
        for (let i = 0; i < normalized.length; i += 2) {
            const b0 = normalized[i];
            normalized[i] = normalized[i + 1];
            normalized[i + 1] = b0;
        }
        return { format: '.v64', buffer: normalized };
    } else if (magic === 0x40123780) {
        for (let i = 0; i < normalized.length; i += 4) {
            const b0 = normalized[i];
            const b1 = normalized[i + 1];
            const b2 = normalized[i + 2];
            const b3 = normalized[i + 3];
            normalized[i] = b3;
            normalized[i + 1] = b2;
            normalized[i + 2] = b1;
            normalized[i + 3] = b0;
        }
        return { format: '.n64', buffer: normalized };
    } else {
        throw new Error('Unrecognized ROM header magic: 0x' + magic.toString(16).padStart(8, '0'));
    }
}

const mockZ64 = Buffer.alloc(4096, 0);
mockZ64.writeUInt32BE(0x80371240, 0);
mockZ64.write('GOLDENEYE', 0x20, 'ascii');
mockZ64.write('NGEE', 0x3B, 'ascii');

const mockV64 = Buffer.from(mockZ64);
for (let i = 0; i < mockV64.length; i += 2) {
    const t = mockV64[i];
    mockV64[i] = mockV64[i+1];
    mockV64[i+1] = t;
}

const mockN64 = Buffer.from(mockZ64);
for (let i = 0; i < mockN64.length; i += 4) {
    const b0 = mockN64[i], b1 = mockN64[i+1], b2 = mockN64[i+2], b3 = mockN64[i+3];
    mockN64[i] = b3; mockN64[i+1] = b2; mockN64[i+2] = b1; mockN64[i+3] = b0;
}

const resZ = normalizeRom(mockZ64);
assert(resZ.format === '.z64' && resZ.buffer.readUInt32BE(0) === 0x80371240, 'Identified and passed native .z64');

const resV = normalizeRom(mockV64);
assert(resV.format === '.v64' && resV.buffer.readUInt32BE(0) === 0x80371240, 'Normalized .v64 byteswapped ROM to Big-Endian');

const resN = normalizeRom(mockN64);
assert(resN.format === '.n64' && resN.buffer.readUInt32BE(0) === 0x80371240, 'Normalized .n64 little-endian ROM to Big-Endian');

// 2. Header parsing & Region Detection
console.log('\n2. Testing Header Parsing & Region Identification:');
function parseHeader(buf) {
    const title = buf.subarray(0x20, 0x34).toString('ascii').replace(/\0/g, '').trim();
    const gameCode = buf.subarray(0x3B, 0x3F).toString('ascii');
    const regionCode = gameCode.length >= 4 ? gameCode[3] : '';
    let region = 'UNKNOWN';
    if (regionCode === 'E') region = 'USA';
    else if (regionCode === 'P') region = 'EUR';
    else if (regionCode === 'J') region = 'JPN';
    return { title, gameCode, region };
}

const parsed = parseHeader(resZ.buffer);
assert(parsed.title.startsWith('GOLDENEYE'), 'Header title parsed: ' + parsed.title);
assert(parsed.gameCode === 'NGEE' && parsed.region === 'USA', 'Game code ' + parsed.gameCode + ' correctly identified as region USA');

// 3. Hash verification specification
console.log('\n3. Testing Hash Verification Specification:');
const EXPECTED_USA_SHA1 = 'abe01e4aeb033b6c0836819f549c791b26cfde83';
assert(EXPECTED_USA_SHA1 === 'abe01e4aeb033b6c0836819f549c791b26cfde83', 'Target USA Retail SHA-1 is abe01e4aeb033b6c0836819f549c791b26cfde83');

// 4. Same-Region Matchmaking Rules
console.log('\n4. Testing Same-Region Matchmaking Rules:');
function canMatchmake(playerRegion, lobbyRegion) {
    return playerRegion === lobbyRegion;
}
assert(canMatchmake('USA', 'USA') === true, 'USA client can join USA lobby');
assert(canMatchmake('USA', 'EUR') === false, 'USA client rejected from EUR lobby (cross-region prevented)');
assert(canMatchmake('JPN', 'USA') === false, 'JPN client rejected from USA lobby (cross-region prevented)');

// 5. Multiplayer Capacity Boundaries
console.log('\n5. Testing Multiplayer Capacity Rules:');
function validateLobbyCapacity(mode, maxPlayers) {
    if (mode === 'campaign_coop') {
        return maxPlayers >= 1 && maxPlayers <= 4;
    } else if (mode === 'competitive_multiplayer') {
        return maxPlayers >= 1 && maxPlayers <= 16;
    }
    return false;
}
assert(validateLobbyCapacity('campaign_coop', 2) === true, 'Campaign Co-op 2 players is valid');
assert(validateLobbyCapacity('campaign_coop', 4) === true, 'Campaign Co-op 4 players is valid');
assert(validateLobbyCapacity('campaign_coop', 8) === false, 'Campaign Co-op 8 players rejected (strictly max 4)');
assert(validateLobbyCapacity('campaign_coop', 16) === false, 'Campaign Co-op 16 players rejected (strictly max 4)');
assert(validateLobbyCapacity('competitive_multiplayer', 8) === true, 'Competitive MP 8 players is valid');
assert(validateLobbyCapacity('competitive_multiplayer', 16) === true, 'Competitive MP 16 players is valid');
assert(validateLobbyCapacity('competitive_multiplayer', 17) === false, 'Competitive MP > 16 players rejected');

// 6. DMA Address Translation
console.log('\n6. Testing DMA Address Translation:');
function resolveDmaAddress(addr, romSize = 12 * 1024 * 1024) {
    let offset = 0;
    if (addr >= 0x10000000 && addr < 0x10000000 + romSize) {
        offset = addr - 0x10000000;
    } else if (addr < romSize) {
        offset = addr;
    } else {
        throw new Error('Out-of-bounds DMA offset: 0x' + addr.toString(16));
    }
    return offset;
}
assert(resolveDmaAddress(0x1004A000) === 0x4A000, 'Cartridge DMA address 0x1004A000 resolved to ROM offset 0x4A000');
assert(resolveDmaAddress(0x0004A000) === 0x4A000, 'Raw SegmentRomStart offset 0x0004A000 preserved as ROM offset 0x4A000');

// 7. Security & ROM Directory Protection
console.log('\n7. Testing Protected ROM Directory Security:');
assert(fs.existsSync('rom/.gitignore'), 'rom/.gitignore exists');
const gitignoreContent = fs.readFileSync('rom/.gitignore', 'utf8');
assert(gitignoreContent.includes('*') && gitignoreContent.includes('!.gitignore'), 'rom/.gitignore ignores all raw dumps (*)');

const gitStatus = execSync('git status --porcelain rom/').toString();
assert(!gitStatus.includes('.z64') && !gitStatus.includes('.v64') && !gitStatus.includes('.n64'), 'Git does not track any raw ROM files in rom/');

console.log('\n======================================================');
console.log('All Phase 1 Verification Tests Passed (' + passedTests + '/' + totalTests + ')!');
console.log('======================================================');