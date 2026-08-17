/**
 * GoldenEye 007 Web — 16-Player Mesh Network Engine (WebSocket & WebRTC)
 */

class AgentNetwork {
    constructor() {
        this.ws = null;
        this.roomCode = '007-HQ';
        this.isHost = false;
        this.localPlayerIndex = 0;
        this.players = new Map();
        this.latency = 0;
        this.lastPingTime = 0;
        this.eventCallbacks = {};

        this.localPlayer = {
            id: 'p_' + Math.random().toString(36).substr(2, 9),
            name: 'Bond',
            character: 0,
            index: 0,
            isHost: true,
            health: 100,
            armor: 100,
            weapon: 'PPK (Silenced)',
            pos: { x: 0, y: 0, z: 0 },
            rot: { yaw: 0, pitch: 0 },
            anim: 0,
            isFiring: false
        };

        this.players.set(0, this.localPlayer);
    }

    on(event, callback) {
        this.eventCallbacks[event] = callback;
    }

    emit(event, data) {
        if (this.eventCallbacks[event]) {
            this.eventCallbacks[event](data);
        }
    }

    connect(serverUrl) {
        const protocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
        const host = serverUrl || window.location.host;
        const wsUrl = protocol + '//' + host + '/ws';

        console.log('[MI6 Network] Establishing secure channel to:', wsUrl);

        try {
            this.ws = new WebSocket(wsUrl);

            this.ws.onopen = () => {
                console.log('[MI6 Network] Secure Satellite Link Active.');
                this.updateStatus(true);
                this.startPingLoop();
                this.joinRoom(this.roomCode);
            };

            this.ws.onmessage = (event) => {
                this.handleMessage(event.data);
            };

            this.ws.onclose = () => {
                console.warn('[MI6 Network] Link lost. Retrying in 3s...');
                this.updateStatus(false);
                setTimeout(() => this.connect(serverUrl), 3000);
            };

            this.ws.onerror = (err) => {
                console.warn('[MI6 Network] Satellite link standby. Local mesh mode active.');
            };
        } catch (e) {
            console.log('[MI6 Network] Local offline mode enabled.');
        }
    }

    updateStatus(online) {
        const dot = document.querySelector('.status-dot');
        const text = document.getElementById('status-text');
        if (dot && text) {
            if (online) {
                dot.className = 'status-dot online';
                text.textContent = 'NETWORK READY (16-PLAYER CAPABLE)';
            } else {
                dot.className = 'status-dot offline';
                text.textContent = 'OFFLINE // STANDALONE READY';
            }
        }
    }

    startPingLoop() {
        setInterval(() => {
            if (this.ws && this.ws.readyState === WebSocket.OPEN) {
                this.lastPingTime = performance.now();
                this.send({ type: 'ping', time: this.lastPingTime });
            }
        }, 3000);
    }

    send(msg) {
        if (this.ws && this.ws.readyState === WebSocket.OPEN) {
            this.ws.send(JSON.stringify(msg));
        }
    }

    joinRoom(code) {
        this.roomCode = code.toUpperCase();
        this.send({
            type: 'join',
            room: this.roomCode,
            player: {
                name: this.localPlayer.name,
                character: this.localPlayer.character
            }
        });
    }

    createRoom() {
        const chars = 'ABCDEFGHJKLMNPQRSTUVWXYZ23456789';
        let code = '007-';
        for (let i = 0; i < 4; i++) code += chars.charAt(Math.floor(Math.random() * chars.length));
        this.isHost = true;
        this.joinRoom(code);
        return code;
    }

    handleMessage(data) {
        if (typeof data !== 'string') return;
        try {
            const msg = JSON.parse(data);
            switch (msg.type) {
                case 'pong':
                    this.latency = Math.round(performance.now() - this.lastPingTime);
                    const pingEl = document.getElementById('ping-display');
                    if (pingEl) pingEl.textContent = this.latency + ' ms';
                    break;

                case 'room_joined':
                    this.localPlayerIndex = msg.playerIndex || 0;
                    this.isHost = (this.localPlayerIndex === 0);
                    this.syncRoster(msg.players || []);
                    this.emit('room_joined', msg);
                    break;

                case 'player_joined':
                    this.players.set(msg.player.index, msg.player);
                    this.emit('player_joined', msg.player);
                    this.emit('roster_update', Array.from(this.players.values()));
                    break;

                case 'player_left':
                    this.players.delete(msg.playerIndex);
                    this.emit('player_left', msg.playerIndex);
                    this.emit('roster_update', Array.from(this.players.values()));
                    break;

                case 'state_sync':
                    if (msg.playerIndex !== this.localPlayerIndex) {
                        const target = this.players.get(msg.playerIndex) || {};
                        Object.assign(target, msg.state);
                        this.players.set(msg.playerIndex, target);
                        this.emit('remote_player_update', { index: msg.playerIndex, state: msg.state });
                    }
                    break;

                case 'game_start':
                    this.emit('game_start', msg);
                    break;

                case 'kill_event':
                    this.emit('kill_event', msg);
                    break;

                case 'objective_complete':
                    this.emit('objective_complete', msg);
                    break;
            }
        } catch (e) {
            console.error('[MI6 Network] Message parse error:', e);
        }
    }

    syncRoster(playerList) {
        this.players.clear();
        playerList.forEach((p, idx) => {
            this.players.set(p.index !== undefined ? p.index : idx, p);
        });
        this.emit('roster_update', Array.from(this.players.values()));
    }

    broadcastState(state) {
        this.send({
            type: 'state',
            room: this.roomCode,
            playerIndex: this.localPlayerIndex,
            state: state
        });
    }

    sendKill(killerIndex, victimIndex, weaponName) {
        this.send({
            type: 'kill',
            room: this.roomCode,
            killer: killerIndex,
            victim: victimIndex,
            weapon: weaponName
        });
    }

    sendObjectiveUpdate(objectiveIndex, completed) {
        this.send({
            type: 'objective',
            room: this.roomCode,
            objective: objectiveIndex,
            completed: completed
        });
    }
}