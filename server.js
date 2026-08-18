const http = require('http');
const fs = require('fs');
const path = require('path');

const PORT = process.env.PORT || 3007;

const MIME_TYPES = {
    '.html': 'text/html; charset=utf-8',
    '.js': 'application/javascript; charset=utf-8',
    '.wasm': 'application/wasm',
    '.map': 'application/json',
    '.css': 'text/css; charset=utf-8',
    '.json': 'application/json',
    '.png': 'image/png',
    '.ico': 'image/x-icon'
};

const server = http.createServer((req, res) => {
    // Set CORS and Cross-Origin isolation headers for SharedArrayBuffer / WASM
    res.setHeader('Cross-Origin-Opener-Policy', 'same-origin');
    res.setHeader('Cross-Origin-Embedder-Policy', 'require-corp');
    res.setHeader('Access-Control-Allow-Origin', '*');

    const url = new URL(req.url, http://localhost:\);
    let reqPath = decodeURIComponent(url.pathname);

    // Block any attempt to access /rom/ or retail assets
    if (reqPath.startsWith('/rom') || reqPath.includes('..')) {
        res.writeHead(403, { 'Content-Type': 'text/plain' });
        res.end('Access Forbidden');
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
            res.writeHead(404, { 'Content-Type': 'text/plain' });
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

server.listen(PORT, () => {
    console.log(GoldenEye 007 Authentic Engine Test Server running at http://localhost:\);
});