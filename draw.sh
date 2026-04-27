#!/bin/bash
# Regenerate keymap visualization after editing coron.keymap
set -e
mkdir -p keymap-drawer
keymap parse -z config/coron.keymap > /tmp/coron_parsed.yaml
keymap draw /tmp/coron_parsed.yaml -j config/coron.json -o keymap-drawer/coron.svg

# キーの左上隅にIDX番号を追加
~/.pyenv/versions/3.12.2/bin/python3 - << 'PYEOF'
import xml.etree.ElementTree as ET, re

ET.register_namespace('', 'http://www.w3.org/2000/svg')
ET.register_namespace('xlink', 'http://www.w3.org/1999/xlink')

tree = ET.parse('keymap-drawer/coron.svg')
root = tree.getroot()
ns = 'http://www.w3.org/2000/svg'

for g in root.iter(f'{{{ns}}}g'):
    cls = g.get('class', '')
    m = re.search(r'keypos-(\d+)', cls)
    if not m:
        continue
    rect = g.find(f'{{{ns}}}rect')
    if rect is None:
        continue
    x = float(rect.get('x', -26))
    y = float(rect.get('y', -26))
    t = ET.SubElement(g, f'{{{ns}}}text')
    t.set('x', str(x + 4))
    t.set('y', str(y + 10))
    t.set('class', 'key-idx')
    t.text = m.group(1)

style_el = root.find(f'{{{ns}}}style')
if style_el is not None and style_el.text:
    style_el.text += '\n.key-idx { font-size: 9px; fill: #888; font-family: monospace; }'

tree.write('keymap-drawer/coron.svg', xml_declaration=True, encoding='unicode')
PYEOF

echo "✓ keymap-drawer/coron.svg updated"
open keymap-drawer/coron.svg
