#!/usr/bin/env bash

set -u

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

echo "============================================================"
echo " ANSA KERNEL - PHASE 9 DOCUMENTATION CHECK"
echo "============================================================"
echo

FAILED=0

check_file() {
    if [ -f "$1" ]; then
        echo "[OK] $1"
    else
        echo "[FAIL] Missing: $1"
        FAILED=1
    fi
}

check_dir() {
    if [ -d "$1" ]; then
        echo "[OK] $1/"
    else
        echo "[FAIL] Missing directory: $1/"
        FAILED=1
    fi
}

echo "[1] Checking directories"
check_dir Docs
check_dir Docs/api
check_dir Docs/architecture
check_dir Docs/development
check_dir Docs/testing
check_dir Docs/release
check_dir tools
check_dir reports

echo
echo "[2] Checking release files"
check_file VERSION
check_file CHANGELOG.md

echo
echo "[3] Checking documentation"
check_file Docs/README.md
check_file Docs/api/API_REFERENCE.md
check_file Docs/architecture/ARCHITECTURE.md
check_file Docs/architecture/ISR_RULES.md
check_file Docs/development/BUILD.md
check_file Docs/testing/TESTING.md
check_file Docs/release/RELEASE_CHECKLIST.md

echo
echo "[4] Checking documentation content"

for file in \
    Docs/README.md \
    Docs/api/API_REFERENCE.md \
    Docs/architecture/ARCHITECTURE.md \
    Docs/architecture/ISR_RULES.md \
    Docs/development/BUILD.md \
    Docs/testing/TESTING.md \
    Docs/release/RELEASE_CHECKLIST.md
do
    if [ -s "$file" ]; then
        echo "[OK] Non-empty: $file"
    else
        echo "[FAIL] Empty: $file"
        FAILED=1
    fi
done

echo

if [ "$FAILED" -eq 0 ]; then
    echo "============================================================"
    echo " PASS: PHASE 9 DOCUMENTATION CHECK"
    echo "============================================================"
    exit 0
else
    echo "============================================================"
    echo " FAIL: PHASE 9 DOCUMENTATION CHECK"
    echo "============================================================"
    exit 1
fi
