#!/usr/bin/env python3
import subprocess


def main():
    output = subprocess.run(
        ["git", "log", "-1", "--format=%an%x00%at%x00%B"],
        capture_output=True,
        text=True,
        check=True,
    ).stdout

    author, timestamp, message = output.split("\0", 2)

    print(author)
    print(int(timestamp) * 1000)
    print(message.strip())


if __name__ == "__main__":
    main()
