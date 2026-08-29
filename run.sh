#!/usr/bin/env bash
#
# run.sh
#
# Demonstrates the simulator in several scenarios, once under EACH
# signature algorithm setup.sh generated a real key/signature set for:
#   1. happy path        -- full valid chain, boots to APP
#   2. corrupted BL2     -- HSM rejects it, chain halts
#   3. wrong-key HSM fw  -- ROM rejects it, HSM never starts
#   4. rollback attack   -- older APP version rejected
#
# After each destructive scenario it restores the good artifacts by
# re-running setup only for the affected file, so scenarios are
# independent. The anti-rollback counters are per-component, not
# per-algorithm (exactly like a real hardware counter would be), so the
# RSA pass's rollback scenario inherits whatever baseline the ECDSA pass
# left behind -- that's intentional, not a bug.
#
set -uo pipefail
cd "$(dirname "$0")"

SIM=bin/secure_boot_sim

if [ ! -x "$SIM" ]; then
    echo "Simulator not built. Run: make"
    exit 1
fi

hr() { echo "============================================================"; }

# $1 = human label, $2 = SECURE_BOOT_ALGO value, $3 = filename suffix ("" or "_rsa")
run_scenarios() {
    local label="$1" algo_env="$2" sfx="$3"

    hr; echo "ALGORITHM: $label"; hr

    echo ""
    hr; echo "SCENARIO 1: happy path (everything valid)"; hr
    SECURE_BOOT_ALGO="$algo_env" "$SIM" || true

    # --- scenario 2: corrupted BL2 --------------------------------------
    hr; echo "SCENARIO 2: corrupted BL2 (HSM must reject)"; hr
    cp images/bl2.bin images/bl2.bin.bak
    cp images/bl2_corrupt.bin images/bl2.bin
    SECURE_BOOT_ALGO="$algo_env" "$SIM" || true
    mv images/bl2.bin.bak images/bl2.bin
    echo "(restored good BL2)"

    # --- scenario 3: wrong-key HSM firmware -----------------------------
    hr; echo "SCENARIO 3: HSM firmware signed with WRONG key ($label; ROM must reject)"; hr
    cp "images/hsm_fw${sfx}.sig" "images/hsm_fw${sfx}.sig.bak"
    # Re-sign the HSM firmware with the attacker key -> ROM's root key won't verify it.
    python3 tools/sign_image.py images/hsm_fw.bin "keys/signing_test_keys/attacker_priv${sfx}.pem" "images/hsm_fw${sfx}.sig" >/dev/null
    SECURE_BOOT_ALGO="$algo_env" "$SIM" || true
    mv "images/hsm_fw${sfx}.sig.bak" "images/hsm_fw${sfx}.sig"
    echo "(restored good HSM firmware signature)"

    # --- scenario 4: rollback attack ------------------------------------
    hr; echo "SCENARIO 4: rollback attack ($label; downgrade APP to older version)"; hr
    echo "First, accept a NEW app version (v2) so the baseline moves forward..."
    cp images/app.bin images/app.bin.bak
    cp "images/app${sfx}.sig" "images/app${sfx}.sig.bak"
    cp images/app_manifest.json images/app_manifest.json.bak

    # Build + sign an APP v2 and run once so the counter advances to 2.
    printf 'APPLICATION-v2 :: simulated trusted application payload (updated)\n' > images/app.bin
    python3 tools/sign_image.py images/app.bin "keys/oem_app_priv${sfx}.pem" "images/app${sfx}.sig" >/dev/null
    python3 tools/make_manifest.py app 2 images/app.bin images/app_manifest.json >/dev/null
    SECURE_BOOT_ALGO="$algo_env" "$SIM" >/dev/null 2>&1 || true
    echo "APP counter is now at v2."

    echo ""
    echo "Now an attacker tries to boot the OLD v1 app (a valid, signed, but"
    echo "downgraded image). Anti-rollback must reject it:"
    mv images/app.bin.bak images/app.bin
    mv "images/app${sfx}.sig.bak" "images/app${sfx}.sig"
    mv images/app_manifest.json.bak images/app_manifest.json
    SECURE_BOOT_ALGO="$algo_env" "$SIM" || true

    echo ""
}

run_scenarios "ECDSA P-256"          "ecdsa" ""
run_scenarios "RSA-2048 / PKCS#1v15" "rsa"   "_rsa"

echo "(note: to reset all counters back to baseline, re-run ./setup.sh)"
hr
echo "All scenarios complete (both algorithms)."
hr
