#!/bin/bash
BASE=~/kron-os-1.0.0-main/kron
RAW=https://raw.githubusercontent.com/MelnikPro/kron-os-1.0.0/main/kron

curl -L "$RAW/compositor/server.c"   -o $BASE/compositor/server.c
curl -L "$RAW/compositor/output.c"   -o $BASE/compositor/output.c
curl -L "$RAW/compositor/toplevel.c" -o $BASE/compositor/toplevel.c
curl -L "$RAW/compositor/toplevel.h" -o $BASE/compositor/toplevel.h

cd $BASE
ninja -C build && echo "=== KRON ГОТОВ ===" || echo "=== ОШИБКИ ==="
