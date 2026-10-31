#!/usr/bin/env python3
import subprocess
import sys
from pathlib import Path

HOOK = """#!/bin/sh
exec python3 .kan/scripts/post-commit.py
"""


def main():
    # hooks may not be in .git/hooks, eg in a worktree or with core.hooksPath set
    hooks_dir = subprocess.run(
        ["git", "rev-parse", "--path-format=absolute", "--git-path", "hooks"],
        cwd=Path(__file__).parent,
        capture_output=True,
        text=True,
        check=True,
    ).stdout.strip()

    hook = Path(hooks_dir) / "post-commit"

    if hook.exists() and hook.read_text() != HOOK:
        sys.exit(f"{hook} already exists with different contents, not overwriting")

    hook.parent.mkdir(parents=True, exist_ok=True)
    hook.write_text(HOOK)
    hook.chmod(0o755)

    print(f"registered {hook}")


if __name__ == "__main__":
    main()
