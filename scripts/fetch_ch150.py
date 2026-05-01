#!/usr/bin/env python3
"""Download the TSPLIB ch150 instance into data/ch150.tsp."""

from __future__ import annotations

import sys
from pathlib import Path
from urllib.error import URLError, HTTPError
from urllib.request import Request, urlopen


URLS = [
    "http://comopt.ifi.uni-heidelberg.de/software/TSPLIB95/tsp/ch150.tsp",
    "https://raw.githubusercontent.com/mastqe/tsplib/master/ch150.tsp",
]


def is_valid_ch150(payload: bytes) -> bool:
    text = payload.decode("utf-8", errors="replace").upper()
    return (
        "NAME: CH150" in text
        and "DIMENSION: 150" in text
        and "NODE_COORD_SECTION" in text
    )


def fetch_payload() -> bytes:
    headers = {"User-Agent": "DualColonyACO/1.0 (Python urllib)"}
    last_error: Exception | None = None

    for url in URLS:
        try:
            request = Request(url, headers=headers)
            with urlopen(request, timeout=30) as response:
                payload = response.read()
            if not is_valid_ch150(payload):
                raise ValueError(f"Downloaded content from {url} is not valid ch150 TSPLIB data.")
            print(f"Downloaded ch150 from: {url}")
            return payload
        except (URLError, HTTPError, TimeoutError, OSError, ValueError) as exc:
            last_error = exc
            print(f"Failed from {url}: {exc}", file=sys.stderr)

    raise RuntimeError(f"Unable to download ch150.tsp from all sources. Last error: {last_error}")


def main() -> int:
    repo_root = Path(__file__).resolve().parents[1]
    target = repo_root / "data" / "ch150.tsp"
    target.parent.mkdir(parents=True, exist_ok=True)

    payload = fetch_payload()
    target.write_bytes(payload)

    print(f"Saved dataset to: {target}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
