#!/bin/bash
# Usage: termshot.sh "<command to run>" /path/to/output.png [wait_seconds]
CMD="$1"
OUT="$2"
WAIT="${3:-1.5}"

osascript <<APPLESCRIPT
tell application "Terminal"
    activate
    do script "clear; $CMD"
    set bounds of front window to {40, 40, 1180, 620}
end tell
APPLESCRIPT

sleep "$WAIT"

WID=$(osascript -e 'tell application "Terminal" to id of front window')
screencapture -x -l"$WID" "$OUT"
echo "Saved $OUT"
