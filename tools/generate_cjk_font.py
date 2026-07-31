#!/usr/bin/env python3
"""Generate yoRadio's compact single-stroke 24x24 Chinese playlist font.

The input is a yoRadio tab-separated playlist. Only CJK Unified Ideographs
used by station names are included. Each Unicode character is mapped to one
otherwise-unused byte so yoRadio's byte-oriented ticker remains unchanged.
"""

from __future__ import annotations

import argparse
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont


def glyph_codes(count: int) -> list[int]:
    # Reserve yoRadio's control-font icons: 0x01-0x06 for Wi-Fi strength,
    # 0x07 for separators, and 0x13-0x15 for volume/status symbols.
    available = list(range(0x08, 0x13))
    available += list(range(0x16, 0x1D))
    # CP437 0xF8 is the degree sign used by the weather ticker.
    available += [code for code in range(0x7F, 0x100) if code != 0xF8]
    if count > len(available):
        raise ValueError(f"font needs {count} codes; only {len(available)} are available")
    return available[:count]


def thin_bitmap(bitmap: list[list[int]]) -> list[list[int]]:
    """Apply Zhang-Suen thinning until every stroke is one pixel wide."""
    height = len(bitmap)
    width = len(bitmap[0])
    changed = True
    while changed:
        changed = False
        for phase in (0, 1):
            remove: list[tuple[int, int]] = []
            for y in range(1, height - 1):
                for x in range(1, width - 1):
                    if not bitmap[y][x]:
                        continue
                    neighbors = [
                        bitmap[y - 1][x], bitmap[y - 1][x + 1], bitmap[y][x + 1],
                        bitmap[y + 1][x + 1], bitmap[y + 1][x], bitmap[y + 1][x - 1],
                        bitmap[y][x - 1], bitmap[y - 1][x - 1],
                    ]
                    count = sum(neighbors)
                    transitions = sum(
                        neighbors[index] == 0 and neighbors[(index + 1) % 8] == 1
                        for index in range(8)
                    )
                    if not 2 <= count <= 6 or transitions != 1:
                        continue
                    if phase == 0:
                        protected = neighbors[0] * neighbors[2] * neighbors[4] or \
                                    neighbors[2] * neighbors[4] * neighbors[6]
                    else:
                        protected = neighbors[0] * neighbors[2] * neighbors[6] or \
                                    neighbors[0] * neighbors[4] * neighbors[6]
                    if not protected:
                        remove.append((x, y))
            if remove:
                changed = True
                for x, y in remove:
                    bitmap[y][x] = 0
    return bitmap


def render_glyph(character: str, font: ImageFont.FreeTypeFont, threshold: int) -> list[int]:
    canvas = Image.new("L", (48, 48), 0)
    draw = ImageDraw.Draw(canvas)
    bounds = draw.textbbox((0, 0), character, font=font)
    width = bounds[2] - bounds[0]
    height = bounds[3] - bounds[1]
    x = (48 - width) // 2 - bounds[0]
    y = (48 - height) // 2 - bounds[1]
    draw.text((x, y), character, fill=255, font=font)
    canvas = canvas.resize((24, 24), Image.Resampling.LANCZOS)
    pixels = canvas.load()
    bitmap = [[1 if pixels[x, y] >= threshold else 0 for x in range(24)] for y in range(24)]
    bitmap = thin_bitmap(bitmap)
    rows = []
    for y in range(24):
        row = 0
        for x in range(24):
            if bitmap[y][x]:
                row |= 0x00800000 >> x
        rows.append(row)
    return rows


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("playlist", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument("--font", type=Path, default=Path(r"C:\Windows\Fonts\simsun.ttc"))
    parser.add_argument("--threshold", type=int, default=50,
                        help="monochrome cutoff from 0 to 255; higher values make strokes thinner")
    args = parser.parse_args()

    text = args.playlist.read_text(encoding="utf-8-sig")
    names = [line.split("\t", 1)[0] for line in text.splitlines() if line.strip()]
    characters = sorted({char for name in names for char in name if "\u4e00" <= char <= "\u9fff"})
    codes = glyph_codes(len(characters))
    font = ImageFont.truetype(str(args.font), 42)

    lines = [
        "#ifndef chinese_playlist_font_h",
        "#define chinese_playlist_font_h",
        "",
        "#include <Arduino.h>",
        "",
        f"constexpr uint16_t CJK_GLYPH_COUNT = {len(characters)};",
        "",
        "const uint16_t cjkCodepoints[CJK_GLYPH_COUNT] PROGMEM = {",
    ]
    for offset in range(0, len(characters), 12):
        chunk = characters[offset : offset + 12]
        lines.append("  " + ", ".join(f"0x{ord(char):04X}" for char in chunk) + ",")
    lines += ["};", "", "const uint8_t cjkEncodedBytes[CJK_GLYPH_COUNT] PROGMEM = {"]
    for offset in range(0, len(codes), 16):
        lines.append("  " + ", ".join(f"0x{code:02X}" for code in codes[offset : offset + 16]) + ",")
    lines += ["};", "", "const uint32_t cjkGlyphs[CJK_GLYPH_COUNT][24] PROGMEM = {"]
    for character in characters:
        rows = render_glyph(character, font, args.threshold)
        if not any(rows):
            raise ValueError(
                f"U+{ord(character):04X} {character!r} became blank; lower --threshold"
            )
        bitmap = ", ".join(f"0x{row:06X}" for row in rows)
        lines.append(f"  {{ {bitmap} }}, // U+{ord(character):04X} {character}")
    lines += [
        "};",
        "",
        "int16_t cjkIndexForCodepoint(uint16_t codepoint);",
        "int16_t cjkIndexForEncodedByte(uint8_t encoded);",
        "uint8_t cjkEncodeCodepoint(uint16_t codepoint);",
        "",
        "#endif",
        "",
    ]
    args.output.write_text("\n".join(lines), encoding="utf-8", newline="\n")
    print(f"Generated {len(characters)} glyphs in {args.output}")


if __name__ == "__main__":
    main()
