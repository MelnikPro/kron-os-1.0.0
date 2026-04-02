#!/bin/bash
set -e
BASE=~/kron-os-1.0.0-main/kron

sed -i 's/#include "server.h"/#include "server.h"\n#include <time.h>/' $BASE/compositor/output.c
sed -i 's/setenv("WAYLAND_DISPLAY", socket, true)/setenv("WAYLAND_DISPLAY", socket, 1)/' $BASE/compositor/server.c
sed -i '/#include <unistd.h>/a #include <stdlib.h>' $BASE/compositor/server.c
sed -i 's/double lx, ly;/int lx, ly;/g' $BASE/compositor/toplevel.c

cd $BASE
ninja -C build
echo "=== ГОТОВО ==="
