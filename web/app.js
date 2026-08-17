/**
 * GoldenEye 007 Web — Main Application & WASM Engine Orchestrator
 */

window.GoldenEyeApp = {
    module: null,
    canvas: null,
    gl: null,
    controls: null,
    network: null,
    currentMode: 'coop',
    isRunning: false,
    romData: null,

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

        const stageSelect = document.getElementById('coop-stage-select');
        stageSelect.addEventListener('change', (e) => {
            this.gameState.stageId = e.target.value;
            this.gameState.stageName = e.target.options[e.target.selectedIndex].text.toUpperCase();
        });

        const btnCreate = document.getElementById('btn-create-room');
        btnCreate.addEventListener('click', () => {
            const code = this.network.createRoom();
            document.getElementById('current-room-badge').textContent = 'ROOM: ' + code;
        });

        const btnJoin = document.getElementById('btn-join-room');
        const roomInput = document.getElementById('room-code-input');
        btnJoin.addEventListener('click', () => {
            const code = roomInput.value.trim();
            if (code) {
                this.network.joinRoom(code);
                document.getElementById('current-room-badge').textContent = 'ROOM: ' + code.toUpperCase();
            }
        });

        const nameInput = document.getElementById('agent-codename');
        nameInput.addEventListener('change', (e) => {
            this.network.localPlayer.name = e.target.value;
        });

        const charSelect = document.getElementById('agent-character');
        charSelect.addEventListener('change', (e) => {
            this.network.localPlayer.character = parseInt(e.target.value);
        });

        const btnBuiltin = document.getElementById('btn-use-builtin-assets');
        btnBuiltin.addEventListener('click', () => {
            this.bootWasmEngine(null);
        });

        const btnStart = document.getElementById('btn-start-game');
        btnStart.addEventListener('click', () => {
            this.launchGame();
        });

        const btnFs = document.getElementById('btn-toggle-fullscreen');
        btnFs.addEventListener('click', () => {
            if (!document.fullscreenElement) {
                document.documentElement.requestFullscreen();
            } else {
                document.exitFullscreen();
            }
        });

        const btnReturn = document.getElementById('btn-return-lobby');
        btnReturn.addEventListener('click', () => {
            this.showLobby();
        });
    },

    setupRomLoader: function() {
        const fileInput = document.getElementById('rom-file-input');
        fileInput.addEventListener('change', (e) => {
            if (e.target.files && e.target.files[0]) {
                const file = e.target.files[0];
                const reader = new FileReader();
                reader.onload = (event) => {
                    this.romData = new Uint8Array(event.target.result);
                    console.log('[GoldenEye 007] Loaded ROM: ' + file.name + ' (' + (this.romData.length / 1024 / 1024).toFixed(2) + ' MB)');
                    const indicator = document.getElementById('rom-indicator-text');
                    if (indicator) indicator.textContent = 'ROM Loaded: ' + file.name;
                    this.bootWasmEngine(this.romData);
                };
                reader.readAsArrayBuffer(file);
            }
        });
    },

    setupNetworkEvents: function() {
        this.network.on('roster_update', (roster) => {
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

        this.network.on('remote_player_update', (data) => {
            this.updateRadarBlips();
        });
    },

    bootWasmEngine: function(romBytes) {
        console.log('[GoldenEye 007] Booting WebAssembly Engine Core...');
        const indicator = document.getElementById('rom-indicator-text');
        if (indicator) indicator.textContent = 'Engine Loaded & Ready';

        if (!window.Module) {
            const script = document.createElement('script');
            script.src = 'goldeneye007.js';
            script.onload = () => {
                console.log('[GoldenEye 007] WebAssembly Module loaded.');
            };
            document.body.appendChild(script);
        }
    },

    launchGame: function() {
        console.log('[GoldenEye 007] Launching Mission: ' + this.gameState.stageName + ' (Mode: ' + this.currentMode.toUpperCase() + ')');

        document.getElementById('lobby-screen').classList.remove('active');
        document.getElementById('game-screen').classList.add('active');
        document.getElementById('btn-return-lobby').style.display = 'inline-block';
        document.getElementById('hud-stage-title').textContent = this.gameState.stageName;

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

    takeDamage: function(amount) {
        if (this.gameState.armor > 0) {
            this.gameState.armor = Math.max(0, this.gameState.armor - amount);
        } else {
            this.gameState.health = Math.max(0, this.gameState.health - amount);
        }
        this.updateHUD();

        const flash = document.getElementById('damage-flash');
        if (flash) {
            flash.classList.add('active');
            setTimeout(() => flash.classList.remove('active'), 120);
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

            const pad = this.controls.poll();

            if (window._hal_input_set_pad) {
                window._hal_input_set_pad(0, pad.buttons, pad.stickX, pad.stickY);
            }

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

window.addEventListener('DOMContentLoaded', () => {
    window.GoldenEyeApp.init();
});