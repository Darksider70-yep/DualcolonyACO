#!/usr/bin/env python3
"""Fetch the TSPLIB kroA100 instance into data/kroA100.tsp."""

from __future__ import annotations

import gzip
import re
import sys
import urllib.error
import urllib.request
from pathlib import Path


ROOT_DIR = Path(__file__).resolve().parents[1]
OUTPUT_PATH = ROOT_DIR / "data" / "kroA100.tsp"

# Primary source is the official TSPLIB host (Heidelberg).
# Secondary source is a widely used mirror that republishes TSPLIB files.
SOURCE_URLS = [
    "https://comopt.ifi.uni-heidelberg.de/software/TSPLIB95/tsp/kroA100.tsp.gz",
    "https://raw.githubusercontent.com/mastqe/tsplib/master/kroA100.tsp",
]


def _decode_tsplib_payload(payload: bytes, url: str) -> str:
    if payload[:2] == b"\x1f\x8b" or url.endswith(".gz"):
        payload = gzip.decompress(payload)
    return payload.decode("utf-8", errors="strict")


def _validate_kroa100(tsplib_text: str) -> None:
    lowered = tsplib_text.lower()
    if "node_coord_section" not in lowered:
        raise ValueError("NODE_COORD_SECTION missing")

    name_match = re.search(r"^name\s*:\s*(\S+)", tsplib_text, flags=re.IGNORECASE | re.MULTILINE)
    if not name_match or name_match.group(1).lower() != "kroa100":
        raise ValueError("NAME is not kroA100")

    dim_match = re.search(r"^dimension\s*:\s*(\d+)", tsplib_text, flags=re.IGNORECASE | re.MULTILINE)
    if not dim_match or int(dim_match.group(1)) != 100:
        raise ValueError("DIMENSION is not 100")

    ewt_match = re.search(
        r"^edge_weight_type\s*:\s*(\S+)",
        tsplib_text,
        flags=re.IGNORECASE | re.MULTILINE,
    )
    if not ewt_match or ewt_match.group(1).upper() != "EUC_2D":
        raise ValueError("EDGE_WEIGHT_TYPE is not EUC_2D")


def main() -> int:
    OUTPUT_PATH.parent.mkdir(parents=True, exist_ok=True)

    for url in SOURCE_URLS:
        try:
            request = urllib.request.Request(
                url,
                headers={"User-Agent": "DualColonyACO/1.0 (kroA100 fetcher)"},
            )
            with urllib.request.urlopen(request, timeout=30) as response:
                payload = response.read()

            text = _decode_tsplib_payload(payload, url)
            _validate_kroa100(text)

            OUTPUT_PATH.write_text(text, encoding="utf-8", newline="\n")
            print(f"Downloaded kroA100 dataset from: {url}")
            print(f"Saved to: {OUTPUT_PATH}")
            return 0
        except (
            urllib.error.URLError,
            urllib.error.HTTPError,
            TimeoutError,
            OSError,
            ValueError,
        ) as exc:
            print(f"Failed source: {url} ({exc})", file=sys.stderr)

    print("Unable to fetch a valid kroA100 TSPLIB file from all configured sources.", file=sys.stderr)
    return 1


if __name__ == "__main__":
    raise SystemExit(main())
