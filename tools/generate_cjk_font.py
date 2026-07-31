#!/usr/bin/env python3
"""Generate yoRadio's compact 6x8 Chinese playlist font.

The input is a yoRadio tab-separated playlist. Only CJK Unified Ideographs
used by station names are included. Each Unicode character is mapped to one
otherwise-unused byte so yoRadio's byte-oriented ticker remains unchanged.
"""

from __future__ import annotations

import argparse
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont


def glyph_codes(count: int) -> list[int]:
    # 0x13-0x15 are yoRadio icons. Avoid NUL, BEL and those icon bytes.
    available = [code for code in range(1, 19) if code != 7]
    available += list(range(0x7F, 0x100))
    if count > len(available):
        raise ValueError(f"font needs {count} codes; only {len(available)} are available")
    return available[:count]


def render_glyph(character: str, font: ImageFont.FreeTypeFont, threshold: int) -> list[int]:
    canvas = Image.new("L", (32, 32), 0)
    draw = ImageDraw.Draw(canvas)
    bounds = draw.textbbox((0, 0), character, font=font)
    width = bounds[2] - bounds[0]
    height = bounds[3] - bounds[1]
    x = (32 - width) // 2 - bounds[0]
    y = (32 - height) // 2 - bounds[1]
    draw.text((x, y), character, fill=255, font=font)
    canvas = canvas.resize((6, 8), Image.Resampling.LANCZOS)
    pixels = canvas.load()
    rows = []
    for y in range(8):
        row = 0
        for x in range(6):
            if pixels[x, y] >= threshold:
                row |= 0x80 >> x
        rows.append(row)
    return rows


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("playlist", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument("--font", type=Path, default=Path(r"C:\Windows\Fonts\simsun.ttc"))
    parser.add_argument("--threshold", type=int, default=45,
                        help="monochrome cutoff from 0 to 255; higher values make strokes thinner")
    args = parser.parse_args()

    text = args.playlist.read_text(encoding="utf-8-sig")
    names = [line.split("\t", 1)[0] for line in text.splitlines() if line.strip()]
    characters = sorted({char for name in names for char in name if "\u4e00" <= char <= "\u9fff"})
    codes = glyph_codes(len(characters))
    font = ImageFont.truetype(str(args.font), 28)

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
    lines += ["};", "", "const uint8_t cjkGlyphs[CJK_GLYPH_COUNT][8] PROGMEM = {"]
    for character in characters:
        rows = render_glyph(character, font, args.threshold)
        if not any(rows):
            raise ValueError(
                f"U+{ord(character):04X} {character!r} became blank; lower --threshold"
            )
        bitmap = ", ".join(f"0x{row:02X}" for row in rows)
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
