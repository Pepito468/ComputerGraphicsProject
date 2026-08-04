#!/usr/bin/env bash
./.rebuild.sh

echo "[3/3] Running tests..."
cd build/tests && ctest --output-on-failure
