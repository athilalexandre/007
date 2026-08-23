/**
 * GoldenEye 007 Host-Authoritative WebRTC Netplay Engine
 * 
 * Features:
 * - Real-time WebRTC DataChannel mesh for ultra-low latency netplay
 * - Host-Authoritative 2-Player Co-op Campaign (Dam, Facility, Runway, etc.)
 * - Host-Authoritative 16-Player Online Multiplayer (Temple, Complex, Facility, etc.)
 * - Client-side state smoothing and interpolation
 * - Automatic STUN negotiation and reconnection
 */

class NetplayManager {
    constructor() {
        this.ws = null;
        this.clientId = null;
        this.isHost = false;
        this.roomInfo = null;
        this.peers = new Map(); // peerId -> { pc, dc, ready }
        this.iceServers = [
            { urls: 'stun:stun.l.google.com:19302' },
            { urls: 'stun:stun1.l.google.com:19302' },
            { urls: 'stun:stun2.l.google.com:19302' }
        ];

        // Event callbacks
        this.onRoomCreatedCallback = null;
        this.onRoomJoinedCallback = null;
        this.onPeerJoinedCallback = null;
        this.onPeerLeftCallback = null;
        this.onGameStartedCallback = null;
        this.onStateUpdateCallback = null;
        this.onRemoteEventCallback = null;
        this.onRoomListCallback = null;
        this.onErrorCallback = null;
        this.onChatCallback = null;

        // Authoritative game simulation state (Host)
        this.localInputs = {
            buttons: 0,
            stickX: 0,
            stickY: 0,
            lookPitch: 0,
            lookYaw: 0
        };

        this.remoteInputs = new Map(); // peerId -> input
        this.authoritativePlayers = new Map(); // slot (1..16) -> player state
        this.ping = 0;
        this.lastPingSent = 0;
    }

    connect(serverUrl) {
        if (!serverUrl) {
            const protocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
            serverUrl = `${protocol}//${window.location.host}`;
        }

        return new Promise((resolve, reject) => {
            try {
                this.ws = new WebSocket(serverUrl);

                this.ws.onopen = () => {
                    console.log('Connected to GoldenEye 007 Signaling Server:', serverUrl);
                    this.startPingLoop();
                    resolve();
                };

                this.ws.onmessage = (event) => {
                    try {
                        const msg = JSON.parse(event.data);
                        this.handleSignalingMessage(msg);
                    } catch (e) {
                        console.error('Error parsing signaling message:', e);
                    }
                };

                this.ws.onerror = (err) => {
                    console.error('Signaling WebSocket error:', err);
                    if (this.onErrorCallback) this.onErrorCallback(err);
                };

                this.ws.onclose = () => {
                    console.log('Signaling WebSocket connection closed.');
                };
            } catch (err) {
                reject(err);
            }
        });
    }

    startPingLoop() {
        setInterval(() => {
            if (this.ws && this.ws.readyState === WebSocket.OPEN) {
                this.lastPingSent = performance.now();
                this.sendSignaling({ type: 'ping', time: this.lastPingSent });
            }
        }, 3000);
    }

    sendSignaling(data) {
        if (this.ws && this.ws.readyState === WebSocket.OPEN) {
            this.ws.send(JSON.stringify(data));
        }
    }

    handleSignalingMessage(msg) {
        switch (msg.type) {
            case 'connected':
                this.clientId = msg.clientId;
                break;

            case 'pong':
                this.ping = Math.round(performance.now() - msg.time);
                break;

            case 'room_created':
                this.isHost = true;
                this.roomInfo = msg.roomInfo;
                if (this.onRoomCreatedCallback) this.onRoomCreatedCallback(msg);
                break;

            case 'room_joined':
                this.isHost = false;
                this.roomInfo = msg.roomInfo;
                if (this.onRoomJoinedCallback) this.onRoomJoinedCallback(msg);
                break;

            case 'room_list':
                if (this.onRoomListCallback) this.onRoomListCallback(msg.rooms);
                break;

            case 'peer_joined':
                this.roomInfo = msg.roomInfo;
                if (this.isHost) {
                    // Host initiates WebRTC connection with new peer
                    this.initiatePeerConnection(msg.peer.id);
                }
                if (this.onPeerJoinedCallback) this.onPeerJoinedCallback(msg.peer, msg.roomInfo);
                break;

            case 'peer_left':
                this.roomInfo = msg.roomInfo;
                if (this.peers.has(msg.peerId)) {
                    const peer = this.peers.get(msg.peerId);
                    if (peer.pc) peer.pc.close();
                    this.peers.delete(msg.peerId);
                }
                if (this.onPeerLeftCallback) this.onPeerLeftCallback(msg.peerId, msg.roomInfo);
                break;

            case 'game_started':
                if (this.onGameStartedCallback) this.onGameStartedCallback(msg);
                break;

            case 'signal':
                this.handlePeerSignal(msg.senderId, msg.data);
                break;

            case 'chat':
                if (this.onChatCallback) this.onChatCallback(msg);
                break;

            case 'error':
                if (this.onErrorCallback) this.onErrorCallback(msg.message);
                break;
        }
    }

    // -------------------------------------------------------------------------
    // Room Matchmaking Operations
    // -------------------------------------------------------------------------
    createRoom(options = {}) {
        this.sendSignaling({
            type: 'create_room',
            mode: options.mode || 'coop',
            stage: options.stage || 1,
            maxPlayers: options.maxPlayers || (options.mode === 'coop' ? 2 : 16),
            name: options.name || 'Bond',
            character: options.character || 'bond',
            weaponSet: options.weaponSet || 'pistols',
            scenario: options.scenario || 'deathmatch'
        });
    }

    joinRoom(roomCode, options = {}) {
        this.sendSignaling({
            type: 'join_room',
            roomCode: roomCode,
            name: options.name || 'Agent',
            character: options.character || 'natalya'
        });
    }

    leaveRoom() {
        this.sendSignaling({ type: 'leave_room' });
        this.peers.forEach(peer => {
            if (peer.pc) peer.pc.close();
        });
        this.peers.clear();
        this.roomInfo = null;
        this.isHost = false;
    }

    listRooms() {
        this.sendSignaling({ type: 'list_rooms' });
    }

    startGame() {
        if (!this.isHost) return;
        this.sendSignaling({ type: 'start_game' });
    }

    sendChat(text) {
        this.sendSignaling({ type: 'chat', text });
    }

    // -------------------------------------------------------------------------
    // WebRTC Peer-to-Peer DataChannel Setup
    // -------------------------------------------------------------------------
    async initiatePeerConnection(peerId) {
        const pc = new RTCPeerConnection({ iceServers: this.iceServers });
        const dc = pc.createDataChannel('game', { ordered: false, maxRetransmits: 0 });

        const peerObj = { pc, dc, ready: false };
        this.peers.set(peerId, peerObj);

        this.setupDataChannel(peerId, dc);

        pc.onicecandidate = (event) => {
            if (event.candidate) {
                this.sendSignaling({
                    type: 'signal',
                    targetId: peerId,
                    data: { candidate: event.candidate }
                });
            }
        };

        const offer = await pc.createOffer();
        await pc.setLocalDescription(offer);

        this.sendSignaling({
            type: 'signal',
            targetId: peerId,
            data: { sdp: pc.localDescription }
        });
    }

    async handlePeerSignal(senderId, data) {
        let peerObj = this.peers.get(senderId);

        if (!peerObj) {
            const pc = new RTCPeerConnection({ iceServers: this.iceServers });
            peerObj = { pc, dc: null, ready: false };
            this.peers.set(senderId, peerObj);

            pc.ondatachannel = (event) => {
                peerObj.dc = event.channel;
                this.setupDataChannel(senderId, event.channel);
            };

            pc.onicecandidate = (event) => {
                if (event.candidate) {
                    this.sendSignaling({
                        type: 'signal',
                        targetId: senderId,
                        data: { candidate: event.candidate }
                    });
                }
            };
        }

        const pc = peerObj.pc;

        if (data.sdp) {
            await pc.setRemoteDescription(new RTCSessionDescription(data.sdp));
            if (data.sdp.type === 'offer') {
                const answer = await pc.createAnswer();
                await pc.setLocalDescription(answer);
                this.sendSignaling({
                    type: 'signal',
                    targetId: senderId,
                    data: { sdp: pc.localDescription }
                });
            }
        } else if (data.candidate) {
            try {
                await pc.addIceCandidate(new RTCIceCandidate(data.candidate));
            } catch (e) {
                console.error('Error adding ICE candidate:', e);
            }
        }
    }

    setupDataChannel(peerId, dc) {
        dc.onopen = () => {
            console.log(`WebRTC DataChannel opened with peer ${peerId}`);
            const peer = this.peers.get(peerId);
            if (peer) peer.ready = true;
        };

        dc.onmessage = (event) => {
            try {
                const data = JSON.parse(event.data);
                if (this.isHost) {
                    // Host receives client inputs
                    if (data.type === 'input') {
                        this.remoteInputs.set(peerId, data.input);
                    }
                } else {
                    // Client receives authoritative world snapshots
                    if (data.type === 'snapshot') {
                        if (this.onStateUpdateCallback) {
                            this.onStateUpdateCallback(data.state);
                        }
                    } else if (data.type === 'event') {
                        if (this.onRemoteEventCallback) {
                            this.onRemoteEventCallback(data.event);
                        }
                    }
                }
            } catch (e) {
                console.error('Error parsing DataChannel message:', e);
            }
        };

        dc.onclose = () => {
            console.log(`WebRTC DataChannel closed with peer ${peerId}`);
            const peer = this.peers.get(peerId);
            if (peer) peer.ready = false;
        };
    }

    // -------------------------------------------------------------------------
    // High-Speed 60Hz Netplay Loop
    // -------------------------------------------------------------------------
    sendLocalInput(input) {
        this.localInputs = input;
        if (!this.isHost) {
            // Send input packet to Host
            const hostPeer = this.peers.values().next().value;
            if (hostPeer && hostPeer.dc && hostPeer.dc.readyState === 'open') {
                hostPeer.dc.send(JSON.stringify({
                    type: 'input',
                    input: input,
                    time: performance.now()
                }));
            }
        }
    }

    broadcastSnapshot(state) {
        if (!this.isHost) return;
        const msg = JSON.stringify({ type: 'snapshot', state });
        for (const peer of this.peers.values()) {
            if (peer.dc && peer.dc.readyState === 'open') {
                peer.dc.send(msg);
            }
        }
    }

    broadcastEvent(event) {
        if (!this.isHost) return;
        const msg = JSON.stringify({ type: 'event', event });
        for (const peer of this.peers.values()) {
            if (peer.dc && peer.dc.readyState === 'open') {
                peer.dc.send(msg);
            }
        }
    }
}

if (typeof module !== 'undefined' && module.exports) {
    module.exports = NetplayManager;
}
