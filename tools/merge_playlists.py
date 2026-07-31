#!/usr/bin/env python3
"""Merge yoRadio tab-separated playlists while removing duplicate URLs."""

from __future__ import annotations

import argparse
from pathlib import Path


def read_playlist(path: Path) -> list[tuple[str, str, str]]:
    rows: list[tuple[str, str, str]] = []
    for number, raw_line in enumerate(path.read_text(encoding="utf-8-sig").splitlines(), 1):
        if not raw_line.strip():
            continue
        fields = raw_line.split("\t")
        if len(fields) != 3:
            raise ValueError(f"{path}:{number}: expected 3 tab-separated fields")
        name, url, enabled = (field.strip() for field in fields)
        if not name or not url:
            raise ValueError(f"{path}:{number}: station name and URL are required")
        rows.append((name, url, enabled))
    return rows


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("output", type=Path)
    parser.add_argument("inputs", type=Path, nargs="+")
    args = parser.parse_args()

    merged: list[tuple[str, str, str]] = []
    seen_urls: set[str] = set()
    duplicate_count = 0
    for input_path in args.inputs:
        for row in read_playlist(input_path):
            key = row[1].casefold()
            if key in seen_urls:
                duplicate_count += 1
                continue
            seen_urls.add(key)
            merged.append(row)

    args.output.parent.mkdir(parents=True, exist_ok=True)
    text = "".join("\t".join(row) + "\n" for row in merged)
    args.output.write_text(text, encoding="utf-8", newline="\n")
    print(f"Wrote {len(merged)} stations to {args.output} ({duplicate_count} duplicates removed)")


if __name__ == "__main__":
    main()
