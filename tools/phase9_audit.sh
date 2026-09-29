#!/usr/bin/env bash

set -u

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

echo "============================================================"
echo " ANSA KERNEL - PHASE 9 RELEASE AUDIT"
echo "============================================================"
echo

FAILED=0

echo "[1] Repository"
echo "Root: $ROOT"

if git rev-parse --is-inside-work-tree >/dev/null 2>&1; then
    echo "[OK] Git repository detected"
else
    echo "[WARN] Git repository not detected"
fi

echo
echo "[2] Version"

if [ -f VERSION ]; then
    VERSION="$(tr -d '[:space:]' < VERSION)"
    echo "[OK] VERSION = $VERSION"
else
    echo "[FAIL] VERSION missing"
    FAILED=1
fi

echo
echo "[3] Documentation"

for file in \
    Docs/README.md \
    Docs/api/API_REFERENCE.md \
    Docs/architecture/ARCHITECTURE.md \
    Docs/architecture/ISR_RULES.md \
    Docs/development/BUILD.md \
    Docs/testing/TESTING.md \
    Docs/release/RELEASE_CHECKLIST.md
do
    if [ -f "$file" ]; then
        echo "[OK] $file"
    else
        echo "[FAIL] Missing $file"
        FAILED=1
    fi
done

echo
echo "[4] Tools"

for file in \
    tools/check_phase9_docs.sh \
    tools/phase9_audit.sh
do
    if [ -x "$file" ]; then
        echo "[OK] executable: $file"
    else
        echo "[FAIL] not executable: $file"
        FAILED=1
    fi
done

echo
echo "[5] Source tree sanity"

if [ -d Inc ]; then
    echo "[OK] Inc/"
else
    echo "[WARN] Inc/ not found"
fi

if [ -d Src ]; then
    echo "[OK] Src/"
else
    echo "[WARN] Src/ not found"
fi

if [ -f Makefile ]; then
    echo "[OK] Makefile"
else
    echo "[WARN] Makefile not found"
fi

echo
echo "[6] Git status"

if git rev-parse --is-inside-work-tree >/dev/null 2>&1; then
    git status --short
fi

echo

if [ "$FAILED" -eq 0 ]; then
    echo "============================================================"
    echo " PASS: PHASE 9 RELEASE AUDIT"
    echo "============================================================"
    exit 0
else
    echo "============================================================"
    echo " FAIL: PHASE 9 RELEASE AUDIT"
    echo "============================================================"
    exit 1
fi
