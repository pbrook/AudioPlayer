#! /bin/sh

f="${1:-test.mp3}"
s="${2:-/dev/ttyACM0}"
sox "$f" -t u8 -c 1 -r 11k - > "$s"
