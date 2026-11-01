#!/usr/bin/env python3
import subprocess
import sys


def main():
    commit = subprocess.run(
        ["git", "rev-parse", "HEAD"],
        capture_output=True,
        text=True,
        check=True,
    ).stdout.strip()

    try:
        result = subprocess.run(["kan", "link-commit", commit])
    except FileNotFoundError:
        sys.exit("kan not found on PATH, commit not linked to any jobs")

    sys.exit(result.returncode)


if __name__ == "__main__":
    main()
