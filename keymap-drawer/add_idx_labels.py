#!/usr/bin/env python3
"""SVG の各キーに keypos インデックス番号を右上隅に追加する後処理スクリプト。

keymap-drawer が生成した coron.svg の <g class="key keypos-N"> 要素を探し、
<rect .../> の直後に小さいグレーのテキストラベルを挿入する。

Usage:
    python keymap-drawer/add_idx_labels.py [svg_path]
"""

import re
import sys
from pathlib import Path

SVG_PATH = Path(__file__).parent / "coron.svg"


def add_idx_labels(path: Path) -> int:
    content = path.read_text(encoding="utf-8")

    # <g ...class="key keypos-N"...>\n<rect .../>\n の直後にラベルを挿入
    pattern = r'(<g [^>]*class="[^"]*keypos-(\d+)[^"]*"[^>]*>\n)(<rect [^/]*/>\n)'

    count = 0

    def replacer(m: re.Match) -> str:
        nonlocal count
        count += 1
        group_open = m.group(1)
        idx = m.group(2)
        rect_line = m.group(3)
        label = (
            f'<text x="22" y="-18"'
            f' style="font-size:8px;fill:#aaa;text-anchor:end;dominant-baseline:auto">'
            f'{idx}</text>\n'
        )
        return group_open + rect_line + label

    result = re.sub(pattern, replacer, content)

    if count == 0:
        print("WARNING: keypos パターンが見つかりませんでした。SVG の構造を確認してください。")
        sys.exit(1)

    path.write_text(result, encoding="utf-8")
    print(f"完了: {count} 個のキーに idx ラベルを追加しました → {path}")
    return count


if __name__ == "__main__":
    svg_path = Path(sys.argv[1]) if len(sys.argv) > 1 else SVG_PATH
    add_idx_labels(svg_path)
