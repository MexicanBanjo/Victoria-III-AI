#!/usr/bin/env python3
"""
Minimal stub parser.

Right now this just reads a fake "log" and writes a static decisions JSON,
so the file exists and can be extended later.
"""

import json
import argparse
from pathlib import Path


def parse_logs(input_path: Path, out_path: Path) -> None:
    # For now, ignore the actual log contents and emit a dummy decision snapshot.
    _ = input_path.read_text(encoding="utf-8")

    data = {
        "tick": 0,
        "actions": [
            {
                "type": "build",
                "target": "steel_mill",
                "score": 100.0,
                "projected_profit": 1234.0,
                "reason": {
                    "shortage_reduction": 10,
                    "job_creation": 200,
                    "roi": 2.0,
                },
            }
        ],
    }
    out_path.write_text(json.dumps(data, indent=2), encoding="utf-8")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--input", type=Path, required=True)
    ap.add_argument("--out", type=Path, required=True)
    args = ap.parse_args()
    parse_logs(args.input, args.out)


if __name__ == "__main__":
    main()
