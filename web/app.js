// ==============================================================================
// GoldenEye 007 Authentic Engine Web Frontend Adapter
// ==============================================================================

const EXPECTED_SHA1 = 'abe01e4aeb033b6c0836819f549c791b26cfde83';
const RETAIL_SIZE = 12582912;

let g_Module = null;
let g_IsRunning = false;
let g_FrameCount = 0;
let g_LastFpsTime = performance.now();
let g_FpsFrames = 0;

function log(msg, type = 'info') {
    const box = document.getElementById('logs-box');
    if (!box) return;
    const entry = document.createElement('div');
    entry.className = `log-entry log-${type}`;
    const time = new Date().toISOString().substring(11, 19);
    entry.textContent = `[${time}] ${msg}`;
    box.appendChild(entry);
    box.scrollTop = box.scrollHeight;
}

// Convert bytes to hex SHA-1 using SubtleCrypto
async function computeSHA1(buffer) {
    const hashBuffer = await crypto.subtle.digest('SHA-1', buffer);
    const hashArray = Array.from(new Uint8Array(hashBuffer));
    return hashArray.map(b => b.toString(16).padStart(2, '0')).join('');
}

// Normalize ROM formats (.z64 big endian, .v64 byteswapped, .n64 little endian) to Big Endian
function normalizeRom(buffer) {
    const view = new DataView(buffer);
    const magic = view.getUint32(0, false);
    const u8 = new Uint8Array(buffer);

    if (magic === 0x80371240) {
        log('ROM format identified: .z64 (Native Big-Endian)', 'ok');
        return u8;
    } else if (magic === 0x37804012) {
        log('ROM format identified: .v64 (Byte-swapped BADC) -> Normalizing to Big-Endian', 'info');
        const out = new Uint8Array(u8.length);
        for (let i = 0; i < u8.length; i += 2) {
            out[i] = u8[i + 1];
            out[i + 1] = u8[i];
        }
        return out;
    } else if (magic === 0x40123780) {
        log('ROM format identified: .n64 (Little-Endian DCBA) -> Normalizing to Big-Endian', 'info');
        const out = new Uint8Array(u8.length);
        for (let i = 0; i < u8.length; i += 4) {
            out[i] = u8[i + 3];
            out[i + 1] = u8[i + 2];
            out[i + 2] = u8[i + 1];
            out[i + 3] = u8[i];
        }
        return out;
    } else {
        log('Warning: Unrecognized ROM magic bytes (' + magic.toString(16) + '). Attempting direct load.', 'err');
        return u8;
    }
}

async function handleRomFile(file) {
    log(`Loading ROM file: ${file.name} (${(file.size / (1024*1024)).toFixed(2)} MB)...`, 'info');
    document.getElementById('rom-ingestion-status').textContent = 'Reading file...';

    const arrayBuffer = await file.arrayBuffer();
    const normalizedBytes = normalizeRom(arrayBuffer);

    if (normalizedBytes.length < RETAIL_SIZE) {
        log(`Error: File is smaller than retail GoldenEye 007 USA ROM (${normalizedBytes.length} < ${RETAIL_SIZE})`, 'err');
        document.getElementById('rom-status-tag').className = 'status-tag status-err';
        document.getElementById('rom-status-tag').textContent = 'Invalid Size';
        document.getElementById('rom-ingestion-status').textContent = 'Size mismatch';
        return;
    }

    const sha1 = await computeSHA1(normalizedBytes.buffer);
    log(`Calculated SHA-1: ${sha1}`, sha1 === EXPECTED_SHA1 ? 'ok' : 'info');

    if (sha1 === EXPECTED_SHA1) {
        log('Verified authentic GoldenEye 007 USA Retail ROM.', 'ok');
    } else {
        log(`Notice: ROM SHA-1 differs from retail USA baseline (${EXPECTED_SHA1}).`, 'info');
    }

    // Allocate buffer in WASM memory and copy ROM
    const romSize = normalizedBytes.length;
    const ptr = g_Module._malloc(romSize);
    g_Module.HEAPU8.set(normalizedBytes, ptr);

    log('Ingesting ROM into Web Libultra PI bus...', 'info');
    g_Module._hal_os_set_rom_data(ptr, romSize);
    g_Module._free(ptr);

    const romStatus = g_Module._hal_os_get_rom_status();
    if (romStatus === 1) {
        log('ROM mounted successfully. Initializing GoldenEye C decompilation engine...', 'ok');
        document.getElementById('rom-status-tag').className = 'status-tag status-ok';
        document.getElementById('rom-status-tag').textContent = 'Mounted';
        document.getElementById('rom-ingestion-status').textContent = '12.0 MB Mounted';
        document.getElementById('drop-overlay').classList.add('hidden');

        startEngine();
    } else {
        log('Engine rejected ROM layout validation.', 'err');
        document.getElementById('rom-status-tag').className = 'status-tag status-err';
        document.getElementById('rom-status-tag').textContent = 'Rejected';
        document.getElementById('rom-ingestion-status').textContent = 'Layout Invalid';
    }
}

function startEngine() {
    log('Calling _hal_engine_init() -> authentic bossInitMainthreadData()...', 'info');
    const initRes = g_Module._hal_engine_init();
    if (initRes !== 0) {
        log(`Engine initialization failed with code: ${initRes}`, 'err');
        document.getElementById('engine-lifecycle').textContent = 'Init Failed';
        return;
    }

    log('Authentic decompiled engine initialized! Starting frame loop.', 'ok');
    document.getElementById('engine-lifecycle').textContent = 'Running';
    g_IsRunning = true;

    requestAnimationFrame(renderLoop);
}

function renderLoop() {
    if (!g_IsRunning) return;

    // Step the authentic engine frame loop
    g_Module._hal_engine_step();
    g_FrameCount++;
    g_FpsFrames++;

    // Present authentic 320x240 RGBA5551 framebuffer to canvas
    const fbPtr = g_Module._hal_gfx_get_framebuffer();
    if (fbPtr) {
        const canvas = document.getElementById('game-canvas');
        const ctx = canvas.getContext('2d');
        const imgData = ctx.createImageData(320, 240);
        const data32 = new Uint32Array(imgData.data.buffer);
        const fb16 = new Uint16Array(g_Module.HEAPU8.buffer, fbPtr, 320 * 240);

        for (let i = 0; i < 320 * 240; i++) {
            const p = fb16[i];
            const r = ((p >> 11) & 0x1F) * 255 / 31;
            const g = ((p >> 6) & 0x1F) * 255 / 31;
            const b = ((p >> 1) & 0x1F) * 255 / 31;
            const a = (p & 1) ? 255 : 0;
            data32[i] = (a << 24) | (b << 16) | (g << 8) | r;
        }
        ctx.putImageData(imgData, 0, 0);
    }

    // Telemetry updates
    const now = performance.now();
    if (now - g_LastFpsTime >= 1000) {
        const fps = Math.round((g_FpsFrames * 1000) / (now - g_LastFpsTime));
        document.getElementById('fps-counter').textContent = `${fps} FPS (Frame: ${g_FrameCount})`;
        g_FpsFrames = 0;
        g_LastFpsTime = now;
    }

    requestAnimationFrame(renderLoop);
}

// Input handling
const KEY_MAP = {
    'KeyW': 0x0800, // Up
    'KeyS': 0x0400, // Down
    'KeyA': 0x0200, // Left
    'KeyD': 0x0100, // Right
    'Enter': 0x1000, // Start
    'KeyJ': 0x8000, // A
    'KeyK': 0x4000, // B
    'Space': 0x2000, // Z
    'KeyI': 0x0008, // C-Up
    'KeyK': 0x0004, // C-Down
    'KeyJ': 0x0002, // C-Left
    'KeyL': 0x0001, // C-Right
    'KeyQ': 0x0020, // L
    'KeyE': 0x0010  // R
};

let g_CurrentButtons = 0;
window.addEventListener('keydown', (e) => {
    if (KEY_MAP[e.code]) {
        g_CurrentButtons |= KEY_MAP[e.code];
        if (g_Module && g_Module._hal_input_set_buttons) {
            g_Module._hal_input_set_buttons(g_CurrentButtons, 0, 0);
        }
    }
});

window.addEventListener('keyup', (e) => {
    if (KEY_MAP[e.code]) {
        g_CurrentButtons &= ~KEY_MAP[e.code];
        if (g_Module && g_Module._hal_input_set_buttons) {
            g_Module._hal_input_set_buttons(g_CurrentButtons, 0, 0);
        }
    }
});

// Drag & Drop Setup
const dropOverlay = document.getElementById('drop-overlay');
const fileInput = document.getElementById('rom-file-input');

fileInput.addEventListener('change', (e) => {
    if (e.target.files && e.target.files[0]) {
        handleRomFile(e.target.files[0]);
    }
});

dropOverlay.addEventListener('dragover', (e) => {
    e.preventDefault();
    e.stopPropagation();
});

dropOverlay.addEventListener('drop', (e) => {
    e.preventDefault();
    e.stopPropagation();
    if (e.dataTransfer.files && e.dataTransfer.files[0]) {
        handleRomFile(e.dataTransfer.files[0]);
    }
});

// Initialize Emscripten Module
GoldenEyeModule().then((mod) => {
    g_Module = mod;
    log('WebAssembly binary loaded and instantiated successfully.', 'ok');
    document.getElementById('wasm-status-tag').className = 'status-tag status-ok';
    document.getElementById('wasm-status-tag').textContent = 'Ready';
}).catch((err) => {
    log('Failed to instantiate WebAssembly module: ' + err, 'err');
    document.getElementById('wasm-status-tag').className = 'status-tag status-err';
    document.getElementById('wasm-status-tag').textContent = 'Error';
});