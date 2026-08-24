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

async function loadLocalDevRom() {
    clearError();
    if (!g_Module) {
        showError('WebAssembly runtime is still initializing...');
        return;
    }
    log('Fetching local workspace ROM from /assets/ramrom/GoldenEye 007 (USA).z64...', 'info');
    try {
        const res = await fetch('/assets/ramrom/GoldenEye%20007%20(USA).z64');
        if (!res.ok) throw new Error(`HTTP ${res.status}: ${res.statusText}`);
        const buffer = await res.arrayBuffer();
        const file = new File([buffer], 'GoldenEye 007 (USA).z64', { type: 'application/octet-stream' });
        await handleRomFile(file);
    } catch (err) {
        showError('Could not load local workspace ROM: ' + err.message);
    }
}

let g_Renderer = null;

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

    const canvas = document.getElementById('game-canvas');
    if (window.GBIGLRenderer && !g_Renderer) {
        g_Renderer = new GBIGLRenderer(canvas, g_Module);
    }

    log('Authentic decompiled engine initialized! Starting display list render loop.', 'ok');
    g_IsRunning = true;

    // Trigger authentic transition to Mission 1: Dam
    log('Automatically transitioning to Mission 1: Dam (Byelomorye)...', 'info');
    if (g_Module._bossSetLoadedStage) {
        g_Module._bossSetLoadedStage(1);
    }

    if (g_AnimFrameId) cancelAnimationFrame(g_AnimFrameId);
    g_AnimFrameId = requestAnimationFrame(renderLoop);
}

function renderLoop() {
    if (!g_IsRunning) return;

    try {
        // Step authentic engine frame
        g_Module._hal_engine_step();
        g_FpsFrames++;
    } catch (err) {
        console.error('[Engine Step Error]:', err);
        showError('Engine runtime error: ' + err.message);
        g_IsRunning = false;
        return;
    }

    // Present frame using WebGL 2.0 GBI Graphics Engine
    const fbPtr = g_Module._hal_gfx_get_framebuffer ? g_Module._hal_gfx_get_framebuffer() : 0;
    if (g_Renderer) {
        g_Renderer.beginFrame();
        if (fbPtr) {
            g_Renderer.renderFramebuffer(fbPtr);
        }
        g_Renderer.endFrame();
    } else if (fbPtr) {
        const canvas = document.getElementById('game-canvas');
        const ctx = canvas.getContext('2d');
        if (ctx) {
            const imgData = ctx.createImageData(320, 240);
            const data32 = new Uint32Array(imgData.data.buffer);
            const fb16 = new Uint16Array(g_Module.HEAPU8.buffer, fbPtr, 320 * 240);

            for (let i = 0; i < 320 * 240; i++) {
                const p = fb16[i];
                const r = ((p >> 11) & 0x1F) * 255 / 31;
                const g = ((p >> 6) & 0x1F) * 255 / 31;
                const b = ((p >> 1) & 0x1F) * 255 / 31;
                const a = (p & 1) ? 255 : 255;
                data32[i] = (a << 24) | (b << 16) | (g << 8) | r;
            }
            ctx.putImageData(imgData, 0, 0);
        }
    }

    // Single-Player Live Telemetry updates
    const stage = g_Module._hal_engine_get_stage_num ? g_Module._hal_engine_get_stage_num() : 90;
    const ticks = g_Module._hal_engine_get_frame_count ? g_Module._hal_engine_get_frame_count() : 0;
    document.getElementById('engine-ticks').textContent = ticks;
    document.getElementById('current-stage').textContent = stage === 90 ? 'TITLE (0x5A)' : `Dam (0x01)`;

    if (stage === 1) {
        document.getElementById('mission-name').textContent = 'Mission 1: Dam';
        document.getElementById('mission-status-tag').className = 'status-tag status-ok';
        document.getElementById('mission-status-tag').textContent = 'Dam Active';

        if (g_Module._hal_player_get_pos_x) {
            const px = g_Module._hal_player_get_pos_x();
            const py = g_Module._hal_player_get_pos_y();
            const pz = g_Module._hal_player_get_pos_z();
            document.getElementById('player-pos').textContent = `[${px.toFixed(2)}, ${py.toFixed(2)}, ${pz.toFixed(2)}]`;
        }

        if (g_Module._hal_player_get_health) {
            const hp = Math.round((g_Module._hal_player_get_health() || 0) * 100);
            const arm = Math.round((g_Module._hal_player_get_armor() || 0) * 100);
            document.getElementById('player-vitals').textContent = `${hp}% / ${arm}%`;
        }

        if (g_Module._hal_player_get_weapon) {
            const wepId = g_Module._hal_player_get_weapon();
            const ammo = g_Module._hal_player_get_ammo ? g_Module._hal_player_get_ammo() : 0;
            const wepNames = {
                0: 'Unarmed',
                1: 'PP7 (Special Issue)',
                2: 'PP7 (Silenced)',
                3: 'KF7 Soviet',
                4: 'Klobb',
                5: 'Sniper Rifle'
            };
            document.getElementById('player-weapon').textContent = wepNames[wepId] || `Weapon ID ${wepId}`;
            document.getElementById('player-ammo').textContent = `${ammo} rounds loaded`;
        }

        if (g_Module._hal_engine_get_guard_count) {
            const guards = g_Module._hal_engine_get_guard_count();
            document.getElementById('guard-count').textContent = `${guards} Active Guards`;
        }
    } else {
        document.getElementById('mission-name').textContent = 'Title Screen';
        document.getElementById('mission-status-tag').className = 'status-tag status-idle';
        document.getElementById('mission-status-tag').textContent = 'Inactive';
    }

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

    // Process inputs and feed authentic controller registers before next frame
    updateInputs();

    g_AnimFrameId = requestAnimationFrame(renderLoop);
}

function launchDamMission() {
    if (!g_Module || !g_IsRunning) {
        showError('Please mount a valid ROM first before launching Dam mission.');
        return;
    }
    log('Launching Mission 1: Dam (Byelomorye) via authentic stage transition...', 'info');
    g_Module._bossSetLoadedStage(1);
}

// Input Management & Authentic Controller Translation
const g_Keys = {};
let g_MouseLookX = 0;
let g_MouseLookY = 0;
let g_MouseButtons = 0;

function updateInputs() {
    if (!g_Module || !g_Module._hal_input_set_buttons) return;

    let buttons = 0;
    let stickX = 0;
    let stickY = 0;

    // Movement: WASD
    if (g_Keys['KeyW'] || g_Keys['ArrowUp']) stickY += 70;
    if (g_Keys['KeyS'] || g_Keys['ArrowDown']) stickY -= 70;
    if (g_Keys['KeyD'] || g_Keys['ArrowRight']) stickX += 70;
    if (g_Keys['KeyA'] || g_Keys['ArrowLeft']) stickX -= 70;

    // Clamp stick range (-80 to 80)
    stickX = Math.max(-80, Math.min(80, stickX));
    stickY = Math.max(-80, Math.min(80, stickY));

    // Mouse look offset
    if (g_MouseLookX !== 0 || g_MouseLookY !== 0) {
        stickX = Math.max(-80, Math.min(80, stickX + Math.round(g_MouseLookX * 1.5)));
        stickY = Math.max(-80, Math.min(80, stickY - Math.round(g_MouseLookY * 1.5)));
        g_MouseLookX = 0;
        g_MouseLookY = 0;
    }

    // Fire weapon: Left Click or Space
    if ((g_MouseButtons & 1) || g_Keys['Space']) buttons |= 0x2000; // Z Trigger

    // Aim Sights: Right Click or Q
    if ((g_MouseButtons & 2) || g_Keys['KeyQ']) buttons |= 0x0010; // R Trigger

    // Interact / Open Doors: E or K
    if (g_Keys['KeyE'] || g_Keys['KeyK']) buttons |= 0x4000; // B Button

    // Weapon Switch / Reload: R or J
    if (g_Keys['KeyR'] || g_Keys['KeyJ']) buttons |= 0x8000; // A Button

    // Crouch: Ctrl or C
    if (g_Keys['ControlLeft'] || g_Keys['ControlRight'] || g_Keys['KeyC']) buttons |= 0x0004; // D_CBUTTONS

    // Pause / Watch Menu: Tab or Escape or Enter
    if (g_Keys['Tab'] || g_Keys['Enter']) buttons |= 0x1000; // Start Button

    g_Module._hal_input_set_buttons(buttons, stickX, stickY);
}

window.addEventListener('keydown', (e) => {
    g_Keys[e.code] = true;
    if (e.code === 'Tab') e.preventDefault();
});

window.addEventListener('keyup', (e) => {
    g_Keys[e.code] = false;
});

const gameCanvas = document.getElementById('game-canvas');
gameCanvas.addEventListener('click', () => {
    if (document.pointerLockElement !== gameCanvas) {
        gameCanvas.requestPointerLock();
    }
});

window.addEventListener('mousemove', (e) => {
    if (document.pointerLockElement === gameCanvas) {
        g_MouseLookX += e.movementX;
        g_MouseLookY += e.movementY;
    }
});

window.addEventListener('mousedown', (e) => {
    if (e.button === 0) g_MouseButtons |= 1; // Left
    if (e.button === 2) g_MouseButtons |= 2; // Right
});

window.addEventListener('mouseup', (e) => {
    if (e.button === 0) g_MouseButtons &= ~1;
    if (e.button === 2) g_MouseButtons &= ~2;
});

window.addEventListener('contextmenu', (e) => {
    if (document.pointerLockElement === gameCanvas) {
        e.preventDefault();
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