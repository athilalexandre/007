const http = require('http');
const fs = require('fs');
const path = require('path');
const crypto = require('crypto');

const PORT = process.env.PORT || 3007;

const MIME_TYPES = {
    '.html': 'text/html; charset=utf-8',
    '.js': 'application/javascript; charset=utf-8',
    '.wasm': 'application/wasm',
    '.map': 'application/json',
    '.css': 'text/css; charset=utf-8',
    '.json': 'application/json',
    '.png': 'image/png',
    '.ico': 'image/x-icon',
    '.svg': 'image/svg+xml'
};

// -----------------------------------------------------------------------------
// 1. Room & Matchmaking State (Host-Authoritative 2P Co-op & 16P Multiplayer)
// -----------------------------------------------------------------------------
const rooms = new Map(); // roomCode -> RoomObject
const clients = new Map(); // socketId -> ClientObject

class Client {
    constructor(socket, id) {
        this.socket = socket;
        this.id = id;
        this.roomCode = null;
        this.isHost = false;
        this.playerSlot = 1;
        this.name = 'Agent ' + id.slice(0, 4).toUpperCase();
        this.character = 'bond';
    }

    send(data) {
        try {
            if (this.socket.readyState === 'open' || !this.socket.destroyed) {
                sendWsFrame(this.socket, JSON.stringify(data));
            }
        } catch (e) {
            // ignore send error on closed socket
        }
    }
}

class Room {
    constructor(code, hostClient, mode = 'coop', stage = 1, maxPlayers = 2) {
        this.code = code;
        this.hostId = hostClient.id;
        this.mode = mode; // 'coop' (2P) or 'multiplayer' (up to 16P)
        this.stage = stage;
        this.maxPlayers = mode === 'coop' ? 2 : Math.min(16, maxPlayers || 16);
        this.clients = new Map(); // id -> Client
        this.gameState = 'lobby'; // 'lobby', 'in_game'
        this.weaponSet = 'pistols';
        this.scenario = 'deathmatch';
        this.addClient(hostClient, true);
    }

    addClient(client, isHost = false) {
        client.roomCode = this.code;
        client.isHost = isHost;
        
        // Find first available slot 1..maxPlayers
        const usedSlots = new Set();
        for (const c of this.clients.values()) {
            usedSlots.add(c.playerSlot);
        }
        let slot = 1;
        while (usedSlots.has(slot) && slot <= this.maxPlayers) {
            slot++;
        }
        client.playerSlot = slot;
        this.clients.set(client.id, client);
    }

    removeClient(clientId) {
        this.clients.delete(clientId);
        if (this.clients.size === 0) {
            rooms.delete(this.code);
            return null;
        }
        if (this.hostId === clientId) {
            const nextHost = this.clients.values().next().value;
            if (nextHost) {
                this.hostId = nextHost.id;
                nextHost.isHost = true;
            }
        }
        return this.hostId;
    }

    broadcast(msg, excludeId = null) {
        for (const client of this.clients.values()) {
            if (client.id !== excludeId) {
                client.send(msg);
            }
        }
    }

    getInfo() {
        const playerList = [];
        for (const c of this.clients.values()) {
            playerList.push({
                id: c.id,
                name: c.name,
                character: c.character,
                slot: c.playerSlot,
                isHost: c.isHost
            });
        }
        return {
            code: this.code,
            hostId: this.hostId,
            mode: this.mode,
            stage: this.stage,
            maxPlayers: this.maxPlayers,
            playerCount: this.clients.size,
            gameState: this.gameState,
            weaponSet: this.weaponSet,
            scenario: this.scenario,
            players: playerList
        };
    }
}

// -----------------------------------------------------------------------------
// 2. Zero-Dependency RFC 6455 WebSocket Framing
// -----------------------------------------------------------------------------
function sendWsFrame(socket, payloadText) {
    const payload = Buffer.from(payloadText, 'utf8');
    const length = payload.length;
    let header;

    if (length < 126) {
        header = Buffer.alloc(2);
        header[0] = 0x81; // FIN + text opcode (1)
        header[1] = length;
    } else if (length < 65536) {
        header = Buffer.alloc(4);
        header[0] = 0x81;
        header[1] = 126;
        header.writeUInt16BE(length, 2);
    } else {
        header = Buffer.alloc(10);
        header[0] = 0x81;
        header[1] = 127;
        header.writeBigUInt64BE(BigInt(length), 2);
    }

    socket.write(Buffer.concat([header, payload]));
}

function handleWsConnection(socket, req) {
    const clientId = crypto.randomBytes(6).toString('hex');
    const client = new Client(socket, clientId);
    clients.set(clientId, client);

    let buffer = Buffer.alloc(0);

    socket.on('data', (chunk) => {
        buffer = Buffer.concat([buffer, chunk]);

        while (buffer.length >= 2) {
            const firstByte = buffer[0];
            const secondByte = buffer[1];
            const isFinal = (firstByte & 0x80) !== 0;
            const opcode = firstByte & 0x0f;
            const isMasked = (secondByte & 0x80) !== 0;
            let payloadLen = secondByte & 0x7f;
            let offset = 2;

            if (opcode === 0x08) {
                socket.end();
                return;
            }

            if (payloadLen === 126) {
                if (buffer.length < 4) return;
                payloadLen = buffer.readUInt16BE(2);
                offset = 4;
            } else if (payloadLen === 127) {
                if (buffer.length < 10) return;
                payloadLen = Number(buffer.readBigUInt64BE(2));
                offset = 10;
            }

            let maskKey = null;
            if (isMasked) {
                if (buffer.length < offset + 4) return;
                maskKey = buffer.slice(offset, offset + 4);
                offset += 4;
            }

            if (buffer.length < offset + payloadLen) return;

            let payload = buffer.slice(offset, offset + payloadLen);
            buffer = buffer.slice(offset + payloadLen);

            if (isMasked && maskKey) {
                for (let i = 0; i < payload.length; i++) {
                    payload[i] ^= maskKey[i % 4];
                }
            }

            if (opcode === 0x01) {
                try {
                    const msg = JSON.parse(payload.toString('utf8'));
                    handleSignalingMessage(client, msg);
                } catch (e) {
                    console.error('Invalid JSON WS message:', e);
                }
            } else if (opcode === 0x09) {
                const pongHeader = Buffer.from([0x8a, 0x00]);
                socket.write(pongHeader);
            }
        }
    });

    socket.on('close', () => {
        handleClientDisconnect(client);
    });

    socket.on('error', () => {
        handleClientDisconnect(client);
    });

    client.send({
        type: 'connected',
        clientId: client.id
    });
}

function handleClientDisconnect(client) {
    clients.delete(client.id);
    if (client.roomCode && rooms.has(client.roomCode)) {
        const room = rooms.get(client.roomCode);
        const newHostId = room.removeClient(client.id);
        if (newHostId) {
            room.broadcast({
                type: 'peer_left',
                peerId: client.id,
                roomInfo: room.getInfo()
            });
        }
    }
}

function generateRoomCode() {
    const chars = 'ABCDEFGHJKLMNPQRSTUVWXYZ23456789';
    let code = '';
    for (let i = 0; i < 6; i++) {
        code += chars[Math.floor(Math.random() * chars.length)];
    }
    return code;
}

function handleSignalingMessage(client, msg) {
    const type = msg.type;

    switch (type) {
        case 'create_room': {
            const mode = msg.mode === 'multiplayer' ? 'multiplayer' : 'coop';
            const stage = msg.stage || 1;
            const maxPlayers = mode === 'multiplayer' ? (msg.maxPlayers || 16) : 2;
            const code = generateRoomCode();
            const room = new Room(code, client, mode, stage, maxPlayers);
            if (msg.name) client.name = String(msg.name).slice(0, 16);
            if (msg.character) client.character = msg.character;
            if (msg.weaponSet) room.weaponSet = msg.weaponSet;
            if (msg.scenario) room.scenario = msg.scenario;

            rooms.set(code, room);

            client.send({
                type: 'room_created',
                roomCode: code,
                roomInfo: room.getInfo()
            });
            break;
        }

        case 'join_room': {
            const code = (msg.roomCode || '').toUpperCase().trim();
            if (!rooms.has(code)) {
                client.send({ type: 'error', message: `Room "${code}" not found.` });
                return;
            }
            const room = rooms.get(code);
            if (room.clients.size >= room.maxPlayers) {
                client.send({ type: 'error', message: `Room "${code}" is full (${room.maxPlayers}/${room.maxPlayers}).` });
                return;
            }

            if (msg.name) client.name = String(msg.name).slice(0, 16);
            if (msg.character) client.character = msg.character;

            room.addClient(client, false);

            client.send({
                type: 'room_joined',
                roomCode: code,
                roomInfo: room.getInfo()
            });

            room.broadcast({
                type: 'peer_joined',
                peer: {
                    id: client.id,
                    name: client.name,
                    character: client.character,
                    slot: client.playerSlot,
                    isHost: false
                },
                roomInfo: room.getInfo()
            }, client.id);
            break;
        }

        case 'leave_room': {
            if (client.roomCode && rooms.has(client.roomCode)) {
                const room = rooms.get(client.roomCode);
                room.removeClient(client.id);
                room.broadcast({
                    type: 'peer_left',
                    peerId: client.id,
                    roomInfo: room.getInfo()
                });
                client.roomCode = null;
                client.send({ type: 'room_left' });
            }
            break;
        }

        case 'list_rooms': {
            const list = [];
            for (const r of rooms.values()) {
                list.push(r.getInfo());
            }
            client.send({
                type: 'room_list',
                rooms: list
            });
            break;
        }

        case 'update_settings': {
            if (!client.roomCode || !rooms.has(client.roomCode)) return;
            const room = rooms.get(client.roomCode);
            if (!client.isHost) return;

            if (msg.stage !== undefined) room.stage = msg.stage;
            if (msg.weaponSet) room.weaponSet = msg.weaponSet;
            if (msg.scenario) room.scenario = msg.scenario;

            room.broadcast({
                type: 'settings_updated',
                roomInfo: room.getInfo()
            });
            break;
        }

        case 'start_game': {
            if (!client.roomCode || !rooms.has(client.roomCode)) return;
            const room = rooms.get(client.roomCode);
            if (!client.isHost) return;

            room.gameState = 'in_game';
            room.broadcast({
                type: 'game_started',
                stage: room.stage,
                mode: room.mode,
                weaponSet: room.weaponSet,
                scenario: room.scenario,
                roomInfo: room.getInfo()
            });
            break;
        }

        case 'signal': {
            const targetId = msg.targetId;
            if (!targetId || !clients.has(targetId)) return;
            const targetClient = clients.get(targetId);

            targetClient.send({
                type: 'signal',
                senderId: client.id,
                data: msg.data
            });
            break;
        }

        case 'chat': {
            if (!client.roomCode || !rooms.has(client.roomCode)) return;
            const room = rooms.get(client.roomCode);
            room.broadcast({
                type: 'chat',
                senderId: client.id,
                senderName: client.name,
                text: String(msg.text || '').slice(0, 200),
                time: Date.now()
            });
            break;
        }

        case 'ping': {
            client.send({ type: 'pong', time: msg.time });
            break;
        }
    }
}

// -----------------------------------------------------------------------------
// 3. HTTP Server & Upgrade Request Handler
// -----------------------------------------------------------------------------
const server = http.createServer((req, res) => {
    res.setHeader('Cross-Origin-Opener-Policy', 'same-origin');
    res.setHeader('Cross-Origin-Embedder-Policy', 'require-corp');
    res.setHeader('Access-Control-Allow-Origin', '*');

    const url = new URL(req.url, 'http://localhost:' + PORT);
    let reqPath = decodeURIComponent(url.pathname);

    if (reqPath.startsWith('/assets/ramrom') || reqPath.startsWith('/rom') || reqPath.includes('..')) {
        res.writeHead(403, { 'Content-Type': 'text/plain; charset=utf-8' });
        res.end('403 Forbidden: Direct ROM access is strictly blocked.');
        return;
    }

    let filePath;
    if (reqPath === '/' || reqPath === '/index.html') {
        filePath = path.join(__dirname, 'web', 'index.html');
    } else if (reqPath.startsWith('/build/web/')) {
        filePath = path.join(__dirname, reqPath);
    } else if (reqPath === '/goldeneye007.js' || reqPath === '/goldeneye007.wasm' || reqPath === '/goldeneye007.map') {
        filePath = path.join(__dirname, 'build', 'web', reqPath);
    } else {
        filePath = path.join(__dirname, 'web', reqPath);
    }

    fs.stat(filePath, (err, stats) => {
        if (err || !stats.isFile()) {
            res.writeHead(404, { 'Content-Type': 'text/plain; charset=utf-8' });
            res.end('404 Not Found');
            return;
        }

        const ext = path.extname(filePath).toLowerCase();
        const contentType = MIME_TYPES[ext] || 'application/octet-stream';

        res.writeHead(200, {
            'Content-Type': contentType,
            'Content-Length': stats.size,
            'Cache-Control': 'no-cache'
        });

        const stream = fs.createReadStream(filePath);
        stream.pipe(res);
    });
});

server.on('upgrade', (req, socket, head) => {
    const key = req.headers['sec-websocket-key'];
    if (!key) {
        socket.destroy();
        return;
    }

    const acceptKey = crypto
        .createHash('sha1')
        .update(key + '258EAFA5-E914-47DA-95CA-C5AB0DC85B11')
        .digest('base64');

    const headers = [
        'HTTP/1.1 101 Switching Protocols',
        'Upgrade: websocket',
        'Connection: Upgrade',
        'Sec-WebSocket-Accept: ' + acceptKey,
        '\r\n'
    ];

    socket.write(headers.join('\r\n'));
    handleWsConnection(socket, req);
});

server.listen(PORT, () => {
    console.log('================================================================');
    console.log(`🔫 GOLDENEYE 007 WEB SERVER & SIGNALING RUNNING ON PORT ${PORT}`);
    console.log(`🎮 Web Client: http://localhost:${PORT}`);
    console.log(`🌐 Signaling: ws://localhost:${PORT}`);
    console.log('================================================================');
});
