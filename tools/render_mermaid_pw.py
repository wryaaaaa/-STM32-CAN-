"""Render Mermaid diagrams from HTML to PNG using Playwright."""
from playwright.sync_api import sync_playwright
from pathlib import Path

html_path = Path(__file__).parent.parent / 'docs' / 'diagrams' / 'index.html'

with sync_playwright() as p:
    browser = p.chromium.launch()
    page = browser.new_page(viewport={'width': 1200, 'height': 800})
    abs_path = html_path.resolve()
    url = abs_path.as_uri()
    print(f'Opening: {url}')
    page.goto(url)
    # Wait for Mermaid to render
    page.wait_for_timeout(3000)
    page.wait_for_selector('svg', timeout=10000)

    # Find all mermaid containers and screenshot each
    containers = page.query_selector_all('.mermaid')
    names = ['sensor-node', 'gateway-node', 'system-architecture', 'can-bus']

    for i, (container, name) in enumerate(zip(containers, names)):
        out_path = html_path.parent / f'{name}.png'
        container.screenshot(path=str(out_path))
        print(f'  {name}.png ({out_path.stat().st_size} bytes)')

    browser.close()
    print('Done.')
