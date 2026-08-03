#!/usr/bin/env bash
./.rebuild.sh

EXE=$(grep -m1 '^project(' CMakeLists.txt | sed 's/project(\([^ )]*\).*/\1/')

echo "[3/3] Running $EXE..."
cd build && ./"$EXE"
