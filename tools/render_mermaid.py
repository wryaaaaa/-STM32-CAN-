#!/usr/bin/env python3
"""Render Mermaid .mmd files to PNG using mermaid.ink API."""
import zlib
import base64
import urllib.request
from pathlib import Path

def encode_mermaid(code: str) -> str:
    """Encode mermaid code for mermaid.ink API (pako.deflate raw format)."""
    # Use raw deflate (no zlib header/checksum) to match pako.deflate
    compressor = zlib.compressobj(level=9, wbits=-15)  # raw deflate
    data = compressor.compress(code.encode('utf-8'))
    data += compressor.flush()
    b64 = base64.urlsafe_b64encode(data).decode('ascii').rstrip('=')
    return b64

def render(mmd_path: Path, out_path: Path):
    code = mmd_path.read_text(encoding='utf-8')
    encoded = encode_mermaid(code)
    url = f"https://mermaid.ink/img/{encoded}?type=png"

    print(f"Rendering {mmd_path.name} -> {out_path.name}...")
    req = urllib.request.Request(url, headers={'User-Agent': 'Mozilla/5.0'})
    try:
        with urllib.request.urlopen(req, timeout=30) as resp:
            out_path.write_bytes(resp.read())
        print(f"  OK: {out_path.name} ({out_path.stat().st_size} bytes)")
    except Exception as e:
        print(f"  FAIL: {e}")

if __name__ == '__main__':
    diagrams_dir = Path(__file__).parent.parent / 'docs' / 'diagrams'
    for mmd in sorted(diagrams_dir.glob('*.mmd')):
        png = mmd.with_suffix('.png')
        render(mmd, png)
    print("Done.")
