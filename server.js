const http = require('http');
const fs = require('fs');
const path = require('path');
const crypto = require('crypto');

const PORT = process.env.PORT || 3007;
const WEB_DIR = path.join(__dirname, 'web');

const MIME_TYPES = {
    '.html': 'text/html; charset=utf-8',
    '.js':   'application/javascript; charset=utf-8',
    '.css':  'text/css; charset=utf-8',
    '.wasm': 'application/wasm',
    '.json': 'application/json',
    '.png':  'image/png',
    '.jpg':  'image/jpeg',
    '.ico':  'image/x-icon',
    '.bin':  'application/octet-stream',
    '.z64':  'application/octet-stream'
};

const rooms = new Map();

function getOrCreateRoom(code) {
    if (!rooms.has(code)) {
        rooms.set(code, {
            code: code,
            clients: new Map(),
            nextPlayerIndex: 0
        });
    }
    return rooms.get(code);
}

const server = http.createServer((req, res) => {
    res.setHeader('Cross-Origin-Opener-Policy', 'same-origin');
    res.setHeader('Cross-Origin-Embedder-Policy', 'require-corp');

    let reqPath = req.url.split('?')[0];
    if (reqPath === '/' || reqPath === '') reqPath = '/index.html';

    let filePath = path.join(WEB_DIR, reqPath);
    if (!fs.existsSync(filePath)) {
        filePath = path.join(__dirname, reqPath);
    }

    if (!fs.existsSync(filePath) || fs.statSync(filePath).isDirectory()) {
        res.writeHead(404, { 'Content-Type': 'text/plain' });
        res.end('404 Not Found');
        return;
    }

    const ext = path.extname(filePath).toLowerCase();
    const contentType = MIME_TYPES[ext] || 'application/octet-stream';

    res.writeHead(200, { 'Content-Type': contentType });
    fs.createReadStream(filePath).pipe(res);
});

server.on('upgrade', (req, socket, head) => {
    if (req.url !== '/ws') {
        socket.destroy();
        return;
    }

    const key = req.headers['sec-websocket-key'];
    const acceptKey = crypto
        .createHash('sha1')
        .update(key + '258EAFA5-E914-47DA-95CA-C5AB0DC85B11')
        .digest('base64');

    const headers = [
        'HTTP/1.1 101 Switching Protocols',
        'Upgrade: websocket',
        'Connection: Upgrade',
        'Sec-WebSocket-Accept: ' + acceptKey
    ];

    socket.write(headers.join('\r\n') + '\r\n\r\n');

    let currentRoom = null;
    let playerIndex = -1;

    function sendWs(sock, obj) {
        try {
            const json = Buffer.from(JSON.stringify(obj));
            const len = json.length;
            let header;
            if (len <= 125) {
                header = Buffer.from([0x81, len]);
            } else if (len <= 65535) {
                header = Buffer.alloc(4);
                header[0] = 0x81;
                header[1] = 126;
                header.writeUInt16BE(len, 2);
            } else {
                header = Buffer.alloc(10);
                header[0] = 0x81;
                header[1] = 127;
                header.writeBigUInt64BE(BigInt(len), 2);
            }
            sock.write(Buffer.concat([header, json]));
        } catch (e) {}
    }

    function broadcastToRoom(room, msg, excludeSocket = null) {
        if (!room) return;
        room.clients.forEach((pData, sock) => {
            if (sock !== excludeSocket && !sock.destroyed) {
                sendWs(sock, msg);
            }
        });
    }

    let buffer = Buffer.alloc(0);

    socket.on('data', (chunk) => {
        buffer = Buffer.concat([buffer, chunk]);
        while (buffer.length >= 2) {
            const secondByte = buffer[1];
            const isMasked = (secondByte & 0x80) !== 0;
            let payloadLen = secondByte & 0x7f;
            let offset = 2;

            if (payloadLen === 126) {
                if (buffer.length < 4) break;
                payloadLen = buffer.readUInt16BE(2);
                offset = 4;
            } else if (payloadLen === 127) {
                if (buffer.length < 10) break;
                payloadLen = Number(buffer.readBigUInt64BE(2));
                offset = 10;
            }

            let maskKey = null;
            if (isMasked) {
                if (buffer.length < offset + 4) break;
                maskKey = buffer.slice(offset, offset + 4);
                offset += 4;
            }

            if (buffer.length < offset + payloadLen) break;

            const payload = buffer.slice(offset, offset + payloadLen);
            buffer = buffer.slice(offset + payloadLen);

            if (isMasked && maskKey) {
                for (let i = 0; i < payload.length; i++) {
                    payload[i] ^= maskKey[i % 4];
                }
            }

            try {
                const msg = JSON.parse(payload.toString('utf8'));
                if (msg.type === 'ping') {
                    sendWs(socket, { type: 'pong' });
                } else if (msg.type === 'join') {
                    currentRoom = getOrCreateRoom(msg.room || '007-HQ');
                    if (currentRoom.clients.size >= 16) {
                        sendWs(socket, { type: 'error', message: 'Room full (16 players maximum)' });
                        return;
                    }
                    playerIndex = currentRoom.nextPlayerIndex++;
                    const playerData = {
                        index: playerIndex,
                        name: (msg.player && msg.player.name) || ('Agent ' + (playerIndex + 1)),
                        character: (msg.player && msg.player.character) || 0,
                        isHost: (currentRoom.clients.size === 0)
                    };
                    currentRoom.clients.set(socket, playerData);

                    const playersList = Array.from(currentRoom.clients.values());
                    sendWs(socket, {
                        type: 'room_joined',
                        room: currentRoom.code,
                        playerIndex: playerIndex,
                        players: playersList
                    });

                    broadcastToRoom(currentRoom, {
                        type: 'player_joined',
                        player: playerData
                    }, socket);
                } else if (msg.type === 'state') {
                    if (currentRoom) {
                        broadcastToRoom(currentRoom, {
                            type: 'state_sync',
                            playerIndex: playerIndex,
                            state: msg.state
                        }, socket);
                    }
                } else if (msg.type === 'kill') {
                    if (currentRoom) {
                        broadcastToRoom(currentRoom, {
                            type: 'kill_event',
                            killerName: msg.killer,
                            victimName: msg.victim,
                            weapon: msg.weapon
                        });
                    }
                } else if (msg.type === 'objective') {
                    if (currentRoom) {
                        broadcastToRoom(currentRoom, {
                            type: 'objective_complete',
                            objective: msg.objective,
                            completed: msg.completed
                        });
                    }
                }
            } catch (e) {}
        }
    });

    socket.on('close', () => {
        if (currentRoom && currentRoom.clients.has(socket)) {
            currentRoom.clients.delete(socket);
            broadcastToRoom(currentRoom, {
                type: 'player_left',
                playerIndex: playerIndex
            });
            if (currentRoom.clients.size === 0) {
                rooms.delete(currentRoom.code);
            }
        }
    });

    socket.on('error', () => {});
});

server.listen(PORT, () => {
    console.log('======================================================');
    console.log('  GOLDENEYE 007 WEB SERVER READY');
    console.log('  URL: http://localhost:' + PORT);
    console.log('  Modes: Campaign Co-Op (1-16) & 16-Player Online Multi');
    console.log('======================================================');
});