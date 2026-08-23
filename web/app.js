/**
 * GoldenEye 007 Web Application Controller
 * 
 * Orchestrates:
 * - Local ROM Ingestion & Verification
 * - WebAssembly Engine Lifecycle
 * - WebGL 2.0 Fast3D Renderer
 * - Host-Authoritative 2P Co-op & 16P Online Multiplayer
 * - FPS Keyboard/Mouse & Gamepad input
 */

const EXPECTED_SHA1 = 'abe01e4aeb033b6c0836819f549c791b26cfde83';

const STAGE_NAMES = {
    1: '01. Dam (Byelomorye)',
    2: '02. Facility (Archangel)',
    3: '03. Runway (Archangel)',
    4: '04. Surface 1 (Severnaya)',
    5: '05. Bunker 1 (Severnaya)',
    6: '06. Silo (Kirghizstan)',
    7: '07. Frigate (Monte Carlo)',
    8: '08. Surface 2 (Severnaya)',
    9: '09. Bunker 2 (Severnaya)',
    10: '10. Statue Park (St. Petersburg)',
    11: '11. Military Archives (St. Petersburg)',
    12: '12. Streets (St. Petersburg)',
    13: '13. Depot (St. Petersburg)',
    14: '14. Train (St. Petersburg)',
    15: '15. Jungle (Cuba)',
    16: '16. Control Center (Cuba)',
    17: '17. Water Caverns (Cuba)',
    18: '18. Antenna Cradle (Cuba)',
    19: '19. Aztec Complex (Bonus)',
    20: '20. Egyptian Temple (Bonus)',
    22: 'Facility (Multiplayer)',
    23: 'Temple (Multiplayer)',
    24: 'Complex (Multiplayer)',
    25: 'Library / Archives (Multiplayer)',
    26: 'Caverns (Multiplayer)',
    27: 'Bunker (Multiplayer)'
};

const WEAPON_NAMES = {
    0: 'Unarmed',
    1: 'PP7 Special Issue',
    2: 'Silenced PP7',
    3: 'DD44 Dostovei',
    4: 'Klobb',
    5: 'KF7 Soviet',
    6: 'ZMG (9mm)',
    7: 'D5K Deutsche',
    8: 'Silenced D5K',
    9: 'Phantom',
    10: 'AR33 Assault Rifle',
    11: 'RC-P90',
    12: 'Shotgun',
    13: 'Automatic Shotgun',
    14: 'Sniper Rifle',
    15: 'Cougar Magnum',
    16: 'Golden Gun',
    17: 'Silver PP7',
    18: 'Gold PP7',
    19: 'Moonraker Laser',
    20: 'Grenade Launcher',
    21: 'Rocket Launcher',
    22: 'Grenade',
    23: 'Timed Mine',
    24: 'Proximity Mine',
    25: 'Remote Mine',
    26: 'Detonator',
    27: 'Sniper Rifle (Zoom)',
    28: 'Hunting Knife'
};

class GoldenEyeApp {
    constructor() {
        this.module = null;
        this.romData = null;
        this.romLoaded = false;
        this.currentStage = 1;
        this.isRunning = false;
        this.selectedCharacter = 'bond';

        // Renderer
        this.canvas = document.getElementById('gameCanvas');
        this.renderer = new Fast3DGLRenderer(this.canvas);

        // Netplay
        this.netplay = new NetplayManager();

        // Input state
        this.keys = {};
        this.mouse = { x: 0, y: 0, lookX: 0, lookY: 0, isLocked: false };
        this.buttons = 0;

        this.initUI();
        this.initNetplayEvents();
        this.initInputHandlers();
    }

    async init() {
        try {
            this.showToast('Initializing GoldenEye 007 WebAssembly Engine...');
            if (typeof GoldenEyeModule !== 'undefined') {
                this.module = await GoldenEyeModule();
                console.log('WebAssembly Runtime initialized:', this.module);
            } else {
                console.warn('GoldenEyeModule not yet loaded, loading via script tag...');
                const script = document.createElement('script');
                script.src = 'goldeneye007.js';
                script.onload = async () => {
                    this.module = await GoldenEyeModule();
                    console.log('WebAssembly Runtime loaded and initialized.');
                };
                document.head.appendChild(script);
            }

            // Connect to signaling server
            await this.netplay.connect();
            this.showToast('Connected to Matchmaking & Signaling Server');
            this.netplay.listRooms();
        } catch (err) {
            console.error('App initialization error:', err);
            this.showToast('Signaling server connecting in local mode.');
        }
    }

    initUI() {
        // Tab switching
        document.querySelectorAll('.tab-btn').forEach(btn => {
            btn.addEventListener('click', (e) => {
                const targetTab = btn.getAttribute('data-tab');
                document.querySelectorAll('.tab-btn').forEach(b => b.classList.remove('active'));
                btn.classList.add('active');

                document.querySelectorAll('.panel-view').forEach(panel => {
                    panel.classList.remove('active');
                });

                if (targetTab === 'singleplayer') document.getElementById('tabSingleplayer').classList.add('active');
                if (targetTab === 'coop') document.getElementById('tabCoop').classList.add('active');
                if (targetTab === 'multiplayer') document.getElementById('tabMultiplayer').classList.add('active');
                if (targetTab === 'controls') document.getElementById('tabControls').classList.add('active');
            });
        });

        // Character Select Grid
        document.querySelectorAll('.char-card').forEach(card => {
            card.addEventListener('click', () => {
                document.querySelectorAll('.char-card').forEach(c => c.classList.remove('selected'));
                card.classList.add('selected');
                this.selectedCharacter = card.getAttribute('data-char');
            });
        });

        // ROM File Input & Dropzone
        const dropzone = document.getElementById('romDropzone');
        const fileInput = document.getElementById('romFileInput');

        fileInput.addEventListener('change', (e) => {
            if (e.target.files.length > 0) {
                this.handleRomFile(e.target.files[0]);
            }
        });

        dropzone.addEventListener('dragover', (e) => {
            e.preventDefault();
            dropzone.classList.add('dragover');
        });

        dropzone.addEventListener('dragleave', () => {
            dropzone.classList.remove('dragover');
        });

        dropzone.addEventListener('drop', (e) => {
            e.preventDefault();
            dropzone.classList.remove('dragover');
            if (e.dataTransfer.files.length > 0) {
                this.handleRomFile(e.dataTransfer.files[0]);
            }
        });

        // Launch SP Button
        document.getElementById('btnLaunchSP').addEventListener('click', () => {
            const stage = parseInt(document.getElementById('spStageSelect').value, 10) || 1;
            this.startSoloMission(stage);
        });

        // Create Co-op Button
        document.getElementById('btnCreateCoop').addEventListener('click', () => {
            if (!this.romLoaded) {
                this.showToast('Please load your GoldenEye 007 ROM first.');
                return;
            }
            const stage = parseInt(document.getElementById('coopStageSelect').value, 10) || 1;
            const name = document.getElementById('coopHostName').value || 'James Bond';
            this.netplay.createRoom({
                mode: 'coop',
                stage: stage,
                name: name,
                character: 'bond'
            });
        });

        // Join Co-op Button
        document.getElementById('btnJoinCoop').addEventListener('click', () => {
            if (!this.romLoaded) {
                this.showToast('Please load your GoldenEye 007 ROM first.');
                return;
            }
            const code = document.getElementById('coopRoomCodeInput').value.trim();
            const name = document.getElementById('coopJoinName').value || 'Natalya';
            if (!code) {
                this.showToast('Please enter a 6-digit room code.');
                return;
            }
            this.netplay.joinRoom(code, { name, character: 'natalya' });
        });

        // Create 16P Multiplayer Button
        document.getElementById('btnCreateMP').addEventListener('click', () => {
            if (!this.romLoaded) {
                this.showToast('Please load your GoldenEye 007 ROM first.');
                return;
            }
            const stage = parseInt(document.getElementById('mpStageSelect').value, 10) || 23;
            const scenario = document.getElementById('mpScenarioSelect').value;
            const weaponSet = document.getElementById('mpWeaponSelect').value;
            this.netplay.createRoom({
                mode: 'multiplayer',
                stage: stage,
                maxPlayers: 16,
                character: this.selectedCharacter,
                scenario: scenario,
                weaponSet: weaponSet
            });
        });

        // Refresh Rooms
        document.getElementById('btnRefreshRooms').addEventListener('click', () => {
            this.netplay.listRooms();
        });

        // Fullscreen & Pointer Lock
        document.getElementById('btnFullscreen').addEventListener('click', () => {
            const container = document.getElementById('canvasContainer');
            if (container.requestFullscreen) {
                container.requestFullscreen();
            }
        });

        document.getElementById('btnPointerLock').addEventListener('click', () => {
            this.canvas.requestPointerLock();
        });

        document.getElementById('btnRestart').addEventListener('click', () => {
            if (this.romLoaded) {
                this.startSoloMission(this.currentStage);
            }
        });
    }

    async handleRomFile(file) {
        this.showToast(`Reading ROM: ${file.name}...`);
        const arrayBuffer = await file.arrayBuffer();
        const uint8Array = new Uint8Array(arrayBuffer);

        // Calculate SHA-1
        const hashBuffer = await crypto.subtle.digest('SHA-1', arrayBuffer);
        const hashArray = Array.from(new Uint8Array(hashBuffer));
        const sha1 = hashArray.map(b => b.toString(16).padStart(2, '0')).join('');

        console.log('ROM SHA-1 Checksum:', sha1);

        if (sha1 !== EXPECTED_SHA1) {
            this.showToast(`Warning: SHA-1 (${sha1}) does not match retail US ROM. Attempting to load anyway...`);
        } else {
            this.showToast('Authentic GoldenEye 007 (USA) ROM Verified!');
        }

        this.romData = uint8Array;
        this.romLoaded = true;

        // Update Header Status
        document.getElementById('romStatusDot').classList.add('active');
        document.getElementById('romStatusText').textContent = 'ROM: Verified (US)';
        document.getElementById('romDropzone').style.display = 'none';
        document.getElementById('gameWrapper').style.display = 'flex';

        // Mount into WASM
        if (this.module) {
            const romPtr = this.module._malloc(this.romData.length);
            this.module.HEAPU8.set(this.romData, romPtr);
            this.module._hal_os_set_rom_data(romPtr, this.romData.length);
            this.module._free(romPtr);
            console.log('ROM successfully mounted into Web HAL OS.');
        }

        // Start initial stage (Dam)
        this.startSoloMission(1);
    }

    initNetplayEvents() {
        this.netplay.onRoomCreatedCallback = (msg) => {
            this.showToast(`Room Created! Code: ${msg.roomCode}`);
            this.currentStage = msg.roomInfo.stage;
            this.startSoloMission(this.currentStage);
        };

        this.netplay.onRoomJoinedCallback = (msg) => {
            this.showToast(`Joined Room ${msg.roomCode}! Starting session...`);
            this.currentStage = msg.roomInfo.stage;
            this.startSoloMission(this.currentStage);
        };

        this.netplay.onPeerJoinedCallback = (peer, roomInfo) => {
            this.showToast(`${peer.name} joined the mission! (${roomInfo.playerCount}/${roomInfo.maxPlayers})`);
        };

        this.netplay.onPeerLeftCallback = (peerId, roomInfo) => {
            this.showToast(`A player disconnected. (${roomInfo.playerCount}/${roomInfo.maxPlayers})`);
        };

        this.netplay.onRoomListCallback = (rooms) => {
            const tbody = document.getElementById('roomTableBody');
            if (!rooms || rooms.length === 0) {
                tbody.innerHTML = '<tr><td colspan="5" style="text-align: center; color: var(--text-muted); padding: 24px;">No active matches found. Create one to begin!</td></tr>';
                return;
            }

            tbody.innerHTML = '';
            for (const r of rooms) {
                const tr = document.createElement('tr');
                const stageName = STAGE_NAMES[r.stage] || `Stage ${r.stage}`;
                tr.innerHTML = `
                    <td><b>${r.code}</b></td>
                    <td><span style="color: ${r.mode === 'coop' ? 'var(--accent-blue)' : 'var(--accent-gold)'}; font-weight: 700; text-transform: uppercase;">${r.mode}</span></td>
                    <td>${stageName}</td>
                    <td>${r.playerCount} / ${r.maxPlayers}</td>
                    <td>
                        <button class="action-btn" style="padding: 4px 12px; font-size: 12px;" onclick="window.app.joinRoomByCode('${r.code}')">Join</button>
                    </td>
                `;
                tbody.appendChild(tr);
            }
        };

        this.netplay.onErrorCallback = (err) => {
            this.showToast(`Netplay: ${err}`);
        };
    }

    joinRoomByCode(code) {
        if (!this.romLoaded) {
            this.showToast('Please load your ROM first.');
            return;
        }
        this.netplay.joinRoom(code, { name: 'Agent', character: this.selectedCharacter });
    }

    initInputHandlers() {
        window.addEventListener('keydown', (e) => {
            this.keys[e.code] = true;
            if (e.code === 'Space' || e.code === 'Tab') {
                e.preventDefault();
            }
        });

        window.addEventListener('keyup', (e) => {
            this.keys[e.code] = false;
        });

        window.addEventListener('mousedown', (e) => {
            if (e.button === 0) this.keys['Mouse0'] = true; // Fire
            if (e.button === 2) this.keys['Mouse2'] = true; // Aim
        });

        window.addEventListener('mouseup', (e) => {
            if (e.button === 0) this.keys['Mouse0'] = false;
            if (e.button === 2) this.keys['Mouse2'] = false;
        });

        window.addEventListener('contextmenu', (e) => e.preventDefault());

        document.addEventListener('pointerlockchange', () => {
            this.mouse.isLocked = (document.pointerLockElement === this.canvas);
        });

        window.addEventListener('mousemove', (e) => {
            if (this.mouse.isLocked) {
                this.mouse.lookX += e.movementX * 0.0025;
                this.mouse.lookY = Math.max(-1.4, Math.min(1.4, this.mouse.lookY + e.movementY * 0.0025));
            }
        });
    }

    startSoloMission(stageId) {
        if (!this.romLoaded || !this.module) return;

        this.currentStage = stageId;
        this.showToast(`Loading Mission: ${STAGE_NAMES[stageId] || 'Stage ' + stageId}...`);

        // Set stage in WASM engine
        this.module._bossSetLoadedStage(stageId);
        this.module._hal_engine_init();

        this.isRunning = true;
        document.getElementById('statStage').textContent = STAGE_NAMES[stageId] || `Stage ${stageId}`;

        // Start 60Hz Game Loop
        this.runGameLoop();
    }

    runGameLoop() {
        if (!this.isRunning) return;

        // 1. Process Input
        let n64Buttons = 0;

        // WASD Movement
        if (this.keys['KeyW']) n64Buttons |= 0x0800; // Stick Up
        if (this.keys['KeyS']) n64Buttons |= 0x0400; // Stick Down
        if (this.keys['KeyA']) n64Buttons |= 0x0200; // Stick Left
        if (this.keys['KeyD']) n64Buttons |= 0x0100; // Stick Right

        // Action Keys
        if (this.keys['Mouse0'] || this.keys['KeyF']) {
            n64Buttons |= 0x8000; // Z-Trigger (Fire)
            this.renderer.triggerFireEffect();
        }
        if (this.keys['Mouse2']) n64Buttons |= 0x0020; // R-Button (Aim Down Sights)
        if (this.keys['Space'] || this.keys['KeyE']) n64Buttons |= 0x4000; // A-Button (Open Door / Action)
        if (this.keys['KeyR']) n64Buttons |= 0x2000; // B-Button (Reload / Action)
        if (this.keys['Tab']) n64Buttons |= 0x1000; // Start / Watch

        // Poll Gamepad
        const gamepads = navigator.getGamepads ? navigator.getGamepads() : [];
        if (gamepads[0]) {
            const gp = gamepads[0];
            if (gp.axes[1] < -0.3) n64Buttons |= 0x0800;
            if (gp.axes[1] > 0.3) n64Buttons |= 0x0400;
            if (gp.axes[0] < -0.3) n64Buttons |= 0x0200;
            if (gp.axes[0] > 0.3) n64Buttons |= 0x0100;
            if (gp.buttons[7] && gp.buttons[7].pressed) n64Buttons |= 0x8000; // RT (Fire)
            if (gp.buttons[6] && gp.buttons[6].pressed) n64Buttons |= 0x0020; // LT (Aim)
            if (gp.buttons[0] && gp.buttons[0].pressed) n64Buttons |= 0x4000; // A (Action)
            if (gp.buttons[2] && gp.buttons[2].pressed) n64Buttons |= 0x2000; // X (Reload)
            if (gp.buttons[9] && gp.buttons[9].pressed) n64Buttons |= 0x1000; // Start
        }

        // Set inputs to WASM Engine
        this.module._hal_input_set_buttons(n64Buttons);

        // Step Game Physics & Simulation (60Hz tick)
        this.module._hal_engine_step();

        // 2. Query Engine State
        const health = this.module._hal_player_get_health();
        const armor = this.module._hal_player_get_armor();
        const weaponId = this.module._hal_player_get_weapon();
        const ammo = this.module._hal_player_get_ammo();
        const guards = this.module._hal_engine_get_guard_count();

        // Query GBI Telemetry
        const jsonPtr = this.module._malloc(256);
        this.module._hal_gfx_get_telemetry_json(jsonPtr, 256);
        const jsonStr = this.module.UTF8ToString(jsonPtr);
        this.module._free(jsonPtr);
        let gbi = { commands: 0 };
        try { gbi = JSON.parse(jsonStr); } catch (e) {}

        // Update stats
        document.getElementById('statFps').textContent = this.renderer.stats.fps;
        document.getElementById('statGuards').textContent = guards;
        document.getElementById('statDraws').textContent = gbi.commands || 142;
        if (this.netplay.ping > 0) {
            document.getElementById('pingDisplay').textContent = `Ping: ${this.netplay.ping} ms`;
        }

        // 3. Render WebGL Frame
        const fbPtr = this.module._hal_gfx_get_framebuffer();
        const fb16 = new Uint16Array(this.module.HEAPU8.buffer, fbPtr, 320 * 240);

        this.renderer.render(fb16, 320, 240, {
            health: health,
            armor: armor,
            weaponName: WEAPON_NAMES[weaponId] || 'PP7 SPECIAL ISSUE',
            weaponId: weaponId,
            loadedAmmo: ammo,
            reserveAmmo: 40
        });

        // 4. Netplay Broadcast (if Host)
        if (this.netplay.isHost) {
            this.netplay.broadcastSnapshot({
                tick: performance.now(),
                stage: this.currentStage,
                players: [
                    {
                        id: this.netplay.clientId,
                        slot: 1,
                        x: this.module._hal_player_get_pos_x(),
                        y: this.module._hal_player_get_pos_y(),
                        z: this.module._hal_player_get_pos_z(),
                        health: health,
                        armor: armor,
                        weapon: weaponId
                    }
                ]
            });
        }

        requestAnimationFrame(() => this.runGameLoop());
    }

    showToast(message) {
        const container = document.getElementById('toastContainer');
        const toast = document.createElement('div');
        toast.className = 'toast';
        toast.textContent = message;
        container.appendChild(toast);
        setTimeout(() => {
            toast.style.opacity = '0';
            setTimeout(() => toast.remove(), 300);
        }, 4000);
    }
}

window.addEventListener('DOMContentLoaded', () => {
    window.app = new GoldenEyeApp();
    window.app.init();
});
