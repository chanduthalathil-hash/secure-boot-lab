#!/usr/bin/env bash
#
# run_tests.sh
#
# Builds (if needed) and runs all unit tests, reporting a summary.
#
set -uo pipefail
cd "$(dirname "$0")"

make tests >/dev/null || { echo "test build failed"; exit 1; }

TESTS=(
    bin/test_hsm_fw_verify
    bin/test_bl2_verify
    bin/test_app_verify
    bin/test_rollback
)

pass=0
fail=0

for t in "${TESTS[@]}"; do
    echo "------------------------------------------------------------"
    if "$t"; then
        pass=$((pass + 1))
    else
        fail=$((fail + 1))
    fi
done

echo "============================================================"
echo "TEST SUMMARY: $pass passed, $fail failed"
echo "============================================================"

# Rollback test pollutes the state file with a test component; reset it
# so a subsequent ./run.sh starts clean.
if [ -f state/rollback_counters.txt ]; then
    grep -v '^_rollback_test=' state/rollback_counters.txt > state/rollback_counters.tmp || true
    mv state/rollback_counters.tmp state/rollback_counters.txt
fi

[ "$fail" -eq 0 ]
