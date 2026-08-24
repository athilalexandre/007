const http = require('http');
const fs = require('fs');
const path = require('path');

const PORT = 8080;
const WEB_DIR = path.join(__dirname, 'web');
const BUILD_DIR = path.join(__dirname, 'build', 'web');

const MIME_TYPES = {
    '.html': 'text/html; charset=utf-8',
    '.js': 'application/javascript; charset=utf-8',
    '.wasm': 'application/wasm',
    '.css': 'text/css; charset=utf-8',
    '.json': 'application/json; charset=utf-8',
    '.png': 'image/png'
};

const server = http.createServer((req, res) => {
    let reqUrl = decodeURIComponent(req.url.split('?')[0]);
    if (reqUrl === '/') reqUrl = '/index.html';

    let filePath = path.join(WEB_DIR, reqUrl);
    if (!fs.existsSync(filePath)) {
        filePath = path.join(BUILD_DIR, reqUrl);
    }
    if (!fs.existsSync(filePath)) {
        filePath = path.join(__dirname, reqUrl);
    }

    if (!fs.existsSync(filePath) || fs.statSync(filePath).isDirectory()) {
        res.writeHead(404, { 'Content-Type': 'text/plain' });
        res.end('404 Not Found');
        return;
    }

    const ext = path.extname(filePath).toLowerCase();
    const contentType = MIME_TYPES[ext] || 'application/octet-stream';

    res.writeHead(200, {
        'Content-Type': contentType,
        'Cross-Origin-Opener-Policy': 'same-origin',
        'Cross-Origin-Embedder-Policy': 'require-corp'
    });
    fs.createReadStream(filePath).pipe(res);
});

server.listen(PORT, '127.0.0.1', () => {
    console.log(`GoldenEye Web Server running at http://127.0.0.1:${PORT}`);
});