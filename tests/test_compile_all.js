const fs = require('fs');
const path = require('path');
const { execSync } = require('child_process');

function findCFiles(dir, list = []) {
    if (!fs.existsSync(dir)) return list;
    const files = fs.readdirSync(dir);
    for (const f of files) {
        const fullPath = path.join(dir, f).replace(/\\/g, '/');
        const stat = fs.statSync(fullPath);
        if (stat.isDirectory()) {
            if (f !== 'build' && f !== '.git' && f !== 'node_modules') {
                findCFiles(fullPath, list);
            }
        } else if (f.endsWith('.c') && !fullPath.includes('tools/')) {
            list.push(fullPath);
        }
    }
    return list;
}

const cFiles = findCFiles('src').concat(findCFiles('assets'));
console.log('Found ' + cFiles.length + ' C source files.');

let passCount = 0;
let failCount = 0;

for (const f of cFiles) {
    try {
        const cmd = 'docker run --rm -v "C:\\Users\\athil\\Documents\\Projects\\Personal\\007:/src" -w /src emscripten/emsdk:latest emcc -D_LANGUAGE_C -DTARGET_WEB -DVERSION_US -DLANG_US -DREFRESH_NTSC -fms-extensions -Wno-microsoft-anon-tag -I . -I include -I include/PR -I src -I src/game -I src/inflate -I src/platform/include -I assets -c ' + f + ' -o /dev/null';
        execSync(cmd, { stdio: 'pipe' });
        passCount++;
        console.log('  [PASS] ' + f);
    } catch (e) {
        failCount++;
        console.error('  [FAIL] ' + f);
        const errOut = e.stderr ? e.stderr.toString() : (e.stdout ? e.stdout.toString() : e.message);
        console.error(errOut.split('\n').slice(0, 5).join('\n'));
    }
}

console.log('\nResult: ' + passCount + ' passed, ' + failCount + ' failed.');