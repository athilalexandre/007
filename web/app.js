/**
 * GoldenEye 007 Web Ã¢â‚¬â€ Main Application & WASM Engine Orchestrator
 * Connects WebAssembly C Engine (bossEntry, mainproc, lvlStageLoad, hal_engine_step)
 * to HTML5 Canvas RGBA32 Framebuffer and 16-player WebSocket network.
 */

window.GoldenEyeApp = {
    module: null,
    canvas: null,
    ctx: null,
    imgData: null,
    controls: null,
    network: null,
    currentMode: 'coop',
    isRunning: false,
    romData: null,
    romLoaded: false,
    frameCounter: 0,

    gameState: {
        stageId: 'facility',
        stageName: 'FACILITY // CHEMICAL WARFARE',
        difficulty: 1,
        health: 100,
        armor: 100,
        ammoClip: 7,
        ammoReserve: 50,
        weaponName: 'PPK (SILENCED)',
        objectives: [
            { text: 'A. Gain entry to laboratory area', complete: true },
            { text: 'B. Rendezvous with 006', complete: false },
            { text: 'C. Destroy bottling tanks (0/10)', complete: false },
            { text: 'D. Minimize scientist casualties', complete: false }
        ]
    },

    init: async function() {
        console.log('[GoldenEye 007] Initializing Web Environment...');
        this.canvas = document.getElementById('goldeneye-canvas');
        if (this.canvas) {
            this.ctx = this.canvas.getContext('2d', { alpha: false, desynchronized: true });
            this.imgData = this.ctx.createImageData(640, 480);
        }

        this.setupUI();

        this.network = new AgentNetwork();
        this.network.connect();
        this.setupNetworkEvents();

        this.controls = new AgentControls(
            this.canvas,
            () => this.onLocalPlayerFire(),
            () => this.onLocalPlayerAction()
        );

        this.setupRomLoader();
    },

    setupUI: function() {
        const tabCoop = document.getElementById('tab-coop');
        const tabMulti = document.getElementById('tab-multi');
        const panelCoop = document.getElementById('coop-config-panel');
        const panelMulti = document.getElementById('multi-config-panel');

        if (tabCoop && tabMulti && panelCoop && panelMulti) {
            tabCoop.addEventListener('click', () => {
                tabCoop.classList.add('active');
                tabMulti.classList.remove('active');
                panelCoop.classList.remove('hidden');
                panelMulti.classList.add('hidden');
                this.currentMode = 'coop';
            });

            tabMulti.addEventListener('click', () => {
                tabMulti.classList.add('active');
                tabCoop.classList.remove('active');
                panelMulti.classList.remove('hidden');
                panelCoop.classList.add('hidden');
                this.currentMode = 'multi';
            });
        }

        const stageSelect = document.getElementById('coop-stage-select');
        if (stageSelect) {
            stageSelect.addEventListener('change', (e) => {
                this.gameState.stageId = e.target.value;
                this.gameState.stageName = e.target.options[e.target.selectedIndex].text.toUpperCase();
            });
        }

        const btnCreate = document.getElementById('btn-create-room');
        if (btnCreate) {
            btnCreate.addEventListener('click', () => {
                const code = this.network.createRoom();
                document.getElementById('current-room-badge').textContent = 'ROOM: ' + code;
            });
        }

        const btnJoin = document.getElementById('btn-join-room');
        const roomInput = document.getElementById('room-code-input');
        if (btnJoin && roomInput) {
            btnJoin.addEventListener('click', () => {
                const code = roomInput.value.trim();
                if (code) {
                    this.network.joinRoom(code);
                    document.getElementById('current-room-badge').textContent = 'ROOM: ' + code.toUpperCase();
                }
            });
        }

        const nameInput = document.getElementById('agent-codename');
        if (nameInput) {
            nameInput.addEventListener('change', (e) => {
                this.network.localPlayer.name = e.target.value;
            });
        }

        const charSelect = document.getElementById('agent-character');
        if (charSelect) {
            charSelect.addEventListener('change', (e) => {
                this.network.localPlayer.character = parseInt(e.target.value, 10);
            });
        }

        const btnStart = document.getElementById('btn-start-game');
        if (btnStart) {
            btnStart.addEventListener('click', () => {
                this.launchGame();
            });
        }

        const btnReturn = document.getElementById('btn-return-lobby');
        if (btnReturn) {
            btnReturn.addEventListener('click', () => {
                this.showLobby();
            });
        }

        const btnBuiltin = document.getElementById('btn-use-builtin-assets');
        if (btnBuiltin) {
            btnBuiltin.addEventListener('click', () => {
                this.bootEngineStandalone();
            });
        }
    },

    setupRomLoader: function() {
        const fileInput = document.getElementById('rom-file-input');
        const dropZone = document.getElementById('rom-status-box');

        if (fileInput) {
            fileInput.addEventListener('change', (e) => {
                if (e.target.files && e.target.files[0]) {
                    this.loadRomFile(e.target.files[0]);
                }
            });
        }

        if (dropZone) {
            dropZone.addEventListener('dragover', (e) => {
                e.preventDefault();
                dropZone.classList.add('drag-over');
            });
            dropZone.addEventListener('dragleave', () => {
                dropZone.classList.remove('drag-over');
            });
            dropZone.addEventListener('drop', (e) => {
                e.preventDefault();
                dropZone.classList.remove('drag-over');
                if (e.dataTransfer.files && e.dataTransfer.files[0]) {
                    this.loadRomFile(e.dataTransfer.files[0]);
                }
            });
        }
    },

    loadRomFile: async function(file) {
        console.log(`[GoldenEye 007] Reading ROM file: ${file.name} (${file.size} bytes)`);
        const indicator = document.getElementById('rom-indicator-text');
        if (indicator) indicator.textContent = `Validating ${file.name}...`;

        const arrayBuffer = await file.arrayBuffer();
        const uint8 = new Uint8Array(arrayBuffer);

        // Compute SHA-1 hash for verification
        const hashBuf = await crypto.subtle.digest('SHA-1', arrayBuffer);
        const hashArray = Array.from(new Uint8Array(hashBuf));
        const hashHex = hashArray.map(b => b.toString(16).padStart(2, '0')).join('');
        console.log(`[GoldenEye 007] ROM SHA-1: ${hashHex}`);

        const KNOWN_US_V10_SHA1 = 'abe01e4aeb033b6c0836819f549c791b26cfde83';
        let isMatch = (hashHex === KNOWN_US_V10_SHA1);

        // Check format / endianness
        let romData = uint8;
        if (uint8[0] === 0x37 && uint8[1] === 0x80) {
            console.log('[GoldenEye 007] Detected byteswapped .v64 ROM, swapping...');
            romData = new Uint8Array(uint8.length);
            for (let i = 0; i < uint8.length; i += 2) {
                romData[i] = uint8[i + 1];
                romData[i + 1] = uint8[i];
            }
        }

        this.romData = romData;
        this.romLoaded = true;

        if (indicator) {
            indicator.textContent = isMatch
                ? `AUTHENTIC US v1.0 ROM VERIFIED (SHA-1: ${hashHex.substring(0, 8)}...)`
                : `ROM LOADED (${(romData.length / 1048576).toFixed(1)} MB, SHA-1: ${hashHex.substring(0, 8)}...)`;
        }

        this.sendRomToWasm(romData);
    },

    sendRomToWasm: function(romBytes) {
        if (!window.Module || !window.Module._hal_os_set_rom_data) {
            console.warn('[GoldenEye 007] WASM Module not yet ready; caching ROM buffer.');
            return;
        }

        console.log('[GoldenEye 007] Passing ROM buffer to WebAssembly C Engine...');
        const ptr = Module._malloc(romBytes.length);
        Module.HEAPU8.set(romBytes, ptr);
        Module._hal_os_set_rom_data(ptr, romBytes.length);

        if (Module._web_load_rom) {
            Module._web_load_rom(ptr, romBytes.length);
        }
    },

    bootEngineStandalone: function() {
        console.log('[GoldenEye 007] Booting WebAssembly Engine standalone...');
        const indicator = document.getElementById('rom-indicator-text');
        if (indicator) indicator.textContent = 'Engine Core Ready (C Decomp)';

        if (window.Module && Module._web_load_rom) {
            Module._web_load_rom(0, 0);
        }
    },

    setupNetworkEvents: function() {
        this.network.on('room_roster_update', (roster) => {
            const countEl = document.getElementById('agent-count');
            if (countEl) countEl.textContent = roster.length;

            const listEl = document.getElementById('roster-list');
            if (listEl) {
                listEl.innerHTML = '';
                const charNames = ['Bond', 'Natalya', 'Trevelyan', 'Boris', 'Xenia', 'Ourumov', 'Valentin', 'Jaws', 'Oddjob', 'Mayday', 'Samedi', 'Russian Guard', 'Siberian Guard', 'Janus Special', 'Naval Officer', 'Commando'];
                roster.forEach((p, idx) => {
                    const item = document.createElement('div');
                    item.className = 'roster-item' + (p.isHost ? ' host' : '');
                    item.innerHTML = '<span class="player-num">P' + (idx + 1) + '</span>' +
                        '<span class="player-name">' + (p.name || 'Agent') + '</span>' +
                        '<span class="player-character">' + (charNames[p.character] || 'Agent') + '</span>' +
                        (p.isHost ? '<span class="player-badge host-badge">HOST</span>' : '');
                    listEl.appendChild(item);
                });
            }
        });

        this.network.on('kill_event', (data) => {
            this.addKillfeedEntry(data.killerName, data.victimName, data.weapon);
        });

        this.network.on('remote_player_update', () => {
            this.updateRadarBlips();
        });
    },

    launchGame: function() {
        console.log('[GoldenEye 007] Launching Mission: ' + this.gameState.stageName + ' (Mode: ' + this.currentMode.toUpperCase() + ')');

        document.getElementById('lobby-screen').classList.remove('active');
        document.getElementById('game-screen').classList.add('active');
        document.getElementById('btn-return-lobby').style.display = 'inline-block';
        document.getElementById('hud-stage-title').textContent = this.gameState.stageName;

        if (window.Module && Module._lvlStageLoad) {
            console.log('[GoldenEye 007] Invoking original C lvlStageLoad for stage:', this.gameState.stageId);
            // Default to Dam (1) or Facility (2) or Title (0x28)
            Module._lvlStageLoad(0x28);
        }

        this.isRunning = true;
        this.startEngineLoop();
    },

    showLobby: function() {
        this.isRunning = false;
        document.getElementById('game-screen').classList.remove('active');
        document.getElementById('lobby-screen').classList.add('active');
        document.getElementById('btn-return-lobby').style.display = 'none';
    },

    onLocalPlayerFire: function() {
        if (this.gameState.ammoClip > 0) {
            this.gameState.ammoClip--;
            this.updateHUD();
            this.triggerMuzzleFlash();

            this.network.broadcastState({
                isFiring: true,
                ammoClip: this.gameState.ammoClip
            });
        }
    },

    onLocalPlayerAction: function() {
        console.log('[GoldenEye 007] Action Triggered (Door / Interact / Objective)');
    },

    triggerMuzzleFlash: function() {
        const crosshair = document.getElementById('hud-crosshair');
        if (crosshair) {
            crosshair.style.transform = 'translate(-50%, -50%) scale(1.4)';
            setTimeout(() => {
                crosshair.style.transform = 'translate(-50%, -50%) scale(1.0)';
            }, 80);
        }
    },

    addKillfeedEntry: function(killer, victim, weapon) {
        const kf = document.getElementById('hud-killfeed');
        if (!kf) return;
        const entry = document.createElement('div');
        entry.className = 'killfeed-entry';
        entry.textContent = killer + ' [' + weapon + '] ' + victim;
        kf.appendChild(entry);
        setTimeout(() => entry.remove(), 5000);
    },

    updateHUD: function() {
        const healthBar = document.getElementById('hud-health-bar');
        const armorBar = document.getElementById('hud-armor-bar');
        const clipEl = document.getElementById('hud-ammo-clip');
        const reserveEl = document.getElementById('hud-ammo-reserve');

        if (healthBar) healthBar.style.width = this.gameState.health + '%';
        if (armorBar) armorBar.style.width = this.gameState.armor + '%';
        if (clipEl) clipEl.textContent = this.gameState.ammoClip;
        if (reserveEl) reserveEl.textContent = this.gameState.ammoReserve;
    },

    updateRadarBlips: function() {
        const container = document.getElementById('radar-blips-container');
        if (!container) return;
        container.innerHTML = '';

        this.network.players.forEach((p, idx) => {
            if (idx === this.network.localPlayerIndex) return;
            const blip = document.createElement('div');
            blip.style.position = 'absolute';
            blip.style.width = '6px';
            blip.style.height = '6px';
            blip.style.borderRadius = '50%';
            blip.style.background = (this.currentMode === 'coop') ? '#00ff88' : '#ff3344';
            blip.style.boxShadow = '0 0 6px ' + blip.style.background;

            const angle = (idx * (360 / 16)) * (Math.PI / 180);
            const dist = 25 + (idx % 3) * 15;
            const x = 60 + Math.cos(angle) * dist - 3;
            const y = 60 + Math.sin(angle) * dist - 3;

            blip.style.left = x + 'px';
            blip.style.top = y + 'px';
            container.appendChild(blip);
        });
    },

    startEngineLoop: function() {
        const loop = () => {
            if (!this.isRunning) return;

            // 1. Poll inputs
            const pad = this.controls.poll();
            if (window.Module && Module._hal_input_set_pad) {
                Module._hal_input_set_pad(0, pad.buttons, pad.stickX, pad.stickY);
            }

            // 2. Step decompiled C Engine per frame
            if (window.Module && Module._hal_engine_step) {
                Module._hal_engine_step();
            }

            // 3. Rasterize Framebuffer to HTML5 Canvas
            if (window.Module && Module._hal_get_framebuffer && this.ctx && this.imgData) {
                const fbPtr = Module._hal_get_framebuffer();
                if (fbPtr) {
                    const fbBytes = Module.HEAPU8.subarray(fbPtr, fbPtr + (640 * 480 * 4));
                    this.imgData.data.set(fbBytes);
                    this.ctx.putImageData(this.imgData, 0, 0);
                }
            }

            // 4. Update Engine Telemetry
            this.frameCounter++;
            if (this.frameCounter % 60 === 0 && window.Module && Module._hal_get_engine_telemetry) {
                const telemPtr = Module._hal_get_engine_telemetry();
                if (telemPtr) {
                    const telem = new Int32Array(Module.HEAP32.buffer, telemPtr, 5);
                    const [loadedStage, memBankReset, dlCount, gbiCount, engineFrames] = telem;
                    console.log(`[GE007-CORE TELEMETRY] Frame ${engineFrames} | Stage 0x${loadedStage.toString(16)} | Bank ${memBankReset} | DLs: ${dlCount} | GBI Cmds: ${gbiCount}`);
                }
            }

            // 5. Broadcast multiplayer state
            if (Math.random() < 0.33) {
                this.network.broadcastState({
                    health: this.gameState.health,
                    armor: this.gameState.armor,
                    weapon: this.gameState.weaponName
                });
            }

            requestAnimationFrame(loop);
        };
        requestAnimationFrame(loop);
    }
};

// Global Emscripten Module callbacks
window.Module = {
    onRuntimeInitialized: function() {
        console.log('[GE007-CORE] WebAssembly Runtime Initialized. Exported C functions available:');
        console.log(' - bossEntry:', typeof Module._bossEntry);
        console.log(' - mainproc:', typeof Module._mainproc);
        console.log(' - lvlStageLoad:', typeof Module._lvlStageLoad);
        console.log(' - hal_engine_step:', typeof Module._hal_engine_step);
        console.log(' - hal_get_framebuffer:', typeof Module._hal_get_framebuffer);
        console.log(' - hal_get_engine_telemetry:', typeof Module._hal_get_engine_telemetry);

        if (window.GoldenEyeApp && window.GoldenEyeApp.romData) {
            window.GoldenEyeApp.sendRomToWasm(window.GoldenEyeApp.romData);
        }
    }
};

window.addEventListener('DOMContentLoaded', () => {
    window.GoldenEyeApp.init();
});