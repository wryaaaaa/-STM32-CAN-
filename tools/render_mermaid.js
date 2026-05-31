const { execSync } = require('child_process');
const fs = require('fs');
const path = require('path');
const zlib = require('zlib');

const diagramsDir = path.join(__dirname, '..', 'docs', 'diagrams');

// Render each .mmd file using mermaid.ink API (pako.deflate + base64url)
fs.readdirSync(diagramsDir).filter(f => f.endsWith('.mmd')).forEach(file => {
    const mmdPath = path.join(diagramsDir, file);
    const pngPath = mmdPath.replace('.mmd', '.png');
    const code = fs.readFileSync(mmdPath, 'utf-8');

    // pako.deflate = zlib.deflateRaw
    const deflated = zlib.deflateRawSync(Buffer.from(code, 'utf-8'));
    const b64 = deflated.toString('base64')
        .replace(/\+/g, '-')
        .replace(/\//g, '_')
        .replace(/=+$/, '');

    const url = `https://mermaid.ink/img/${b64}?type=png`;
    console.log(`Rendering ${file}...`);

    try {
        // Use curl or PowerShell to download
        const outFile = pngPath.replace(/\\/g, '/');
        execSync(`curl -s -o "${outFile}" "${url}" --max-time 30`, { stdio: 'pipe' });
        const size = fs.statSync(pngPath).size;
        console.log(`  OK: ${path.basename(pngPath)} (${size} bytes)`);
    } catch (e) {
        console.log(`  FAIL: ${e.message}`);
    }
});

console.log('Done.');
