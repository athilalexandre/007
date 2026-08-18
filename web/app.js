// ==============================================================================
// GoldenEye 007 Authentic Engine Web Frontend Adapter
// ==============================================================================

const EXPECTED_SHA1 = 'abe01e4aeb033b6c0836819f549c791b26cfde83';
const EXPECTED_GAME_ID = 'NGEE';
const EXPECTED_SIZE = 12582912; // 12 MB

let g_Module = null;
let g_IsRunning = false;
let g_AnimFrameId = null;
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

function showError(msg) {
    const banner = document.getElementById('error-banner');
    if (banner) {
        banner.textContent = msg;
        banner.className = 'error-banner visible';
    }
    log(msg, 'err');
}

function clearError() {
    const banner = document.getElementById('error-banner');
    if (banner) {
        banner.textContent = '';
        banner.className = 'error-banner';
    }
}

async function computeSHA1(buffer) {
    const hashBuffer = await crypto.subtle.digest('SHA-1', buffer);
    const hashArray = Array.from(new Uint8Array(hashBuffer));
    return hashArray.map(b => b.toString(16).padStart(2, '0')).join('');
}

function normalizeRom(buffer) {
    const view = new DataView(buffer);
    const magic = view.getUint32(0, false);
    const u8 = new Uint8Array(buffer);

    if (magic === 0x80371240) {
        return { data: u8, format: '.z64 (Native Big-Endian)', magic: '0x80371240', ok: true };
    } else if (magic === 0x37804012) {
        const out = new Uint8Array(u8.length);
        for (let i = 0; i < u8.length; i += 2) {
            out[i] = u8[i + 1];
            out[i + 1] = u8[i];
        }
        return { data: out, format: '.v64 (Byte-swapped BADC)', magic: '0x37804012', ok: true };
    } else if (magic === 0x40123780) {
        const out = new Uint8Array(u8.length);
        for (let i = 0; i < u8.length; i += 4) {
            out[i] = u8[i + 3];
            out[i + 1] = u8[i + 2];
            out[i + 2] = u8[i + 1];
            out[i + 3] = u8[i];
        }
        return { data: out, format: '.n64 (Little-Endian DCBA)', magic: '0x40123780', ok: true };
    } else {
        return { data: null, format: 'Unknown', magic: '0x' + magic.toString(16).padStart(8, '0'), ok: false };
    }
}

async function handleRomFile(file) {
    clearError();
    if (!g_Module) {
        showError('Error: WebAssembly module is still loading.');
        return;
    }

    log(`Reading ROM file: ${file.name} (${file.size} bytes)...`, 'info');
    document.getElementById('rom-size').textContent = `${file.size.toLocaleString()} bytes`;

    const arrayBuffer = await file.arrayBuffer();

    if (arrayBuffer.byteLength < EXPECTED_SIZE) {
        showError(`Invalid ROM Size: ${arrayBuffer.byteLength} bytes (Expected: ${EXPECTED_SIZE} bytes for retail USA ROM).`);
        document.getElementById('rom-status-tag').className = 'status-tag status-err';
        document.getElementById('rom-status-tag').textContent = 'Invalid Size';
        return;
    }

    const norm = normalizeRom(arrayBuffer);
    if (!norm.ok) {
        showError(`Unknown ROM magic bytes ${norm.magic}. Expected 0x80371240 (.z64), 0x37804012 (.v64), or 0x40123780 (.n64).`);
        document.getElementById('rom-status-tag').className = 'status-tag status-err';
        document.getElementById('rom-status-tag').textContent = 'Bad Magic';
        return;
    }

    document.getElementById('rom-format').textContent = norm.format;
    log(`Identified format: ${norm.format}`, 'ok');

    const u8 = norm.data;
    let gameId = '';
    for (let i = 0x3B; i <= 0x3E; i++) {
        gameId += String.fromCharCode(u8[i]);
    }
    const version = u8[0x3F];
    document.getElementById('rom-region').textContent = `${gameId} (Revision ${version})`;

    if (gameId !== EXPECTED_GAME_ID) {
        showError('ROM region mismatch. This build currently requires GoldenEye 007 NTSC-U/USA (NGEE). Players in a future online lobby must use the same supported region and revision.');
        document.getElementById('rom-status-tag').className = 'status-tag status-err';
        document.getElementById('rom-status-tag').textContent = 'Bad Region';
        return;
    }

    const sha1 = await computeSHA1(u8.buffer);
    document.getElementById('rom-sha1').textContent = sha1;
    log(`Calculated canonical SHA-1: ${sha1}`, 'info');

    if (sha1 !== EXPECTED_SHA1) {
        showError(`Unsupported ROM Checksum: ${sha1} (Expected canonical USA: ${EXPECTED_SHA1}). Engine will not mount modified or unverified ROM.`);
        document.getElementById('rom-status-tag').className = 'status-tag status-err';
        document.getElementById('rom-status-tag').textContent = 'Bad Hash';
        return;
    }

    log('ROM verified: GoldenEye 007 USA (NTSC-U). Ingesting into WebAssembly memory...', 'ok');

    // Ingest into WASM
    const romSize = u8.length;
    const ptr = g_Module._malloc(romSize);
    g_Module.HEAPU8.set(u8, ptr);
    g_Module._hal_os_set_rom_data(ptr, romSize);
    g_Module._free(ptr);

    const romStatus = g_Module._hal_os_get_rom_status();
    if (romStatus === 1) {
        document.getElementById('rom-status-tag').className = 'status-tag status-ok';
        document.getElementById('rom-status-tag').textContent = 'Mounted';
        document.getElementById('drop-overlay').classList.add('hidden');

        startEngine();
    } else {
        showError('Engine rejected ROM data structure.');
        document.getElementById('rom-status-tag').className = 'status-tag status-err';
        document.getElementById('rom-status-tag').textContent = 'Mount Failed';
    }
}

function startEngine() {
    if (g_IsRunning) {
        log('Engine already running.', 'warn');
        return;
    }

    log('Initializing GoldenEye 007 decompilation engine (_hal_engine_init)...', 'info');
    const res = g_Module._hal_engine_init();
    if (res !== 0 && res !== 1) {
        showError(`Engine initialization failed with code: ${res}`);
        document.getElementById('engine-lifecycle').textContent = `Failed (${res})`;
        document.getElementById('wasm-status-tag').className = 'status-tag status-err';
        return;
    }

    document.getElementById('engine-lifecycle').textContent = 'Running';
    document.getElementById('current-stage').textContent = 'TITLE (0x5A)';
    document.getElementById('wasm-status-tag').className = 'status-tag status-ok';
    document.getElementById('wasm-status-tag').textContent = 'Active';

    log('Authentic decompiled engine initialized! Starting display list render loop.', 'ok');
    g_IsRunning = true;

    if (g_AnimFrameId) cancelAnimationFrame(g_AnimFrameId);
    g_AnimFrameId = requestAnimationFrame(renderLoop);
}

function renderLoop() {
    if (!g_IsRunning) return;

    // Step authentic engine frame
    g_Module._hal_engine_step();
    g_FpsFrames++;

    // Present 320x240 RGBA5551 framebuffer to canvas
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
    const ticks = g_Module._hal_engine_get_frame_count ? g_Module._hal_engine_get_frame_count() : 0;
    const stage = g_Module._hal_engine_get_stage_num ? g_Module._hal_engine_get_stage_num() : 90;
    document.getElementById('engine-ticks').textContent = ticks;
    document.getElementById('current-stage').textContent = stage === 90 ? 'TITLE (0x5A)' : `Stage ${stage}`;

    const dmaCount = g_Module._hal_os_get_dma_transfers ? g_Module._hal_os_get_dma_transfers() : 0;
    const dmaBytes = g_Module._hal_os_get_dma_bytes ? g_Module._hal_os_get_dma_bytes() : 0;
    document.getElementById('dma-transfers').textContent = `${dmaCount.toLocaleString()} reads (${(dmaBytes / 1024).toFixed(1)} KB)`;

    // GBI Telemetry
    if (g_Module._hal_gfx_get_telemetry_json) {
        const jsonPtr = g_Module._malloc(256);
        g_Module._hal_gfx_get_telemetry_json(jsonPtr, 256);
        const jsonStr = g_Module.UTF8ToString(jsonPtr);
        g_Module._free(jsonPtr);
        try {
            const gbi = JSON.parse(jsonStr);
            document.getElementById('gbi-commands').textContent = gbi.commands.toLocaleString();
            document.getElementById('gbi-triangles').textContent = gbi.triangles.toLocaleString();
            document.getElementById('gbi-vertices').textContent = gbi.vertices.toLocaleString();
            document.getElementById('gbi-textures').textContent = gbi.textures.toLocaleString();
            document.getElementById('gbi-unsupported').textContent = gbi.unsupported.toLocaleString();
        } catch (e) {}
    }

    const now = performance.now();
    if (now - g_LastFpsTime >= 1000) {
        const fps = Math.round((g_FpsFrames * 1000) / (now - g_LastFpsTime));
        document.getElementById('fps-counter').textContent = `${fps} FPS \u00B7 ${ticks} Ticks`;
        g_FpsFrames = 0;
        g_LastFpsTime = now;
    }

    g_AnimFrameId = requestAnimationFrame(renderLoop);
}

// Controller Button Mappings (N64 Controller)
const KEY_MAP = {
    'KeyW': 0x0800, // Up
    'KeyS': 0x0400, // Down
    'KeyA': 0x0200, // Left
    'KeyD': 0x0100, // Right
    'Enter': 0x1000, // Start
    'KeyJ': 0x8000, // A
    'KeyK': 0x4000, // B
    'Space': 0x2000, // Z
    'KeyQ': 0x0020, // L
    'KeyE': 0x0010  // R
};

let g_Buttons = 0;
window.addEventListener('keydown', (e) => {
    if (KEY_MAP[e.code]) {
        g_Buttons |= KEY_MAP[e.code];
        if (g_Module && g_Module._hal_input_set_buttons) {
            g_Module._hal_input_set_buttons(g_Buttons, 0, 0);
        }
    }
});

window.addEventListener('keyup', (e) => {
    if (KEY_MAP[e.code]) {
        g_Buttons &= ~KEY_MAP[e.code];
        if (g_Module && g_Module._hal_input_set_buttons) {
            g_Module._hal_input_set_buttons(g_Buttons, 0, 0);
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
    log('WebAssembly binary loaded and initialized successfully.', 'ok');
    document.getElementById('wasm-status-tag').className = 'status-tag status-ok';
    document.getElementById('wasm-status-tag').textContent = 'Ready';
}).catch((err) => {
    showError('Failed to instantiate WebAssembly module: ' + err);
    document.getElementById('wasm-status-tag').className = 'status-tag status-err';
    document.getElementById('wasm-status-tag').textContent = 'Error';
});