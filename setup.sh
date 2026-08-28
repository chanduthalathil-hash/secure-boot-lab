#!/usr/bin/env bash
#
# setup.sh
#
# One-shot setup: generates keys, creates dummy firmware images, signs
# them, produces manifests, and builds the deliberately-broken artifacts
# used by the negative tests.
#
# Run this ONCE after cloning, and again any time you want to reset the
# images/keys to a known-good state. Safe to re-run.
#
# Usage:  ./setup.sh
#
set -euo pipefail

cd "$(dirname "$0")"

echo "############################################################"
echo "# secure_boot_lab setup"
echo "############################################################"

# --- 0. sanity checks -------------------------------------------------
command -v gcc      >/dev/null || { echo "ERROR: gcc not found. Run: sudo apt install build-essential"; exit 1; }
command -v python3  >/dev/null || { echo "ERROR: python3 not found."; exit 1; }
python3 -c "import cryptography" 2>/dev/null || {
    echo "Installing python 'cryptography' package (user)..."
    pip3 install --user cryptography || pip3 install --break-system-packages cryptography
}

mkdir -p images keys/signing_test_keys state bin

# --- 1. reset rollback state -----------------------------------------
# Start each fresh setup with baseline version 1 already accepted for
# every component, so the default happy-path run is a clean success and
# the rollback DEMO (downgrading to an older version) is the thing that
# fails. Without this, the very first run would "accept" whatever version
# it sees as the baseline.
echo ""
echo "[setup] establishing rollback baseline (all components at v1)"
cat > state/rollback_counters.txt <<EOF
hsm_fw=1
bl2=1
app=1
EOF

# --- 2. generate keys -------------------------------------------------
echo ""
echo "[setup] generating keys"
python3 tools/generate_keys.py

# --- 3. create dummy firmware images ----------------------------------
# Real images would be compiled firmware binaries. For the lab, the
# CONTENT doesn't matter -- only that each stage has some bytes to hash
# and sign. We make them distinct and human-recognizable.
echo ""
echo "[setup] creating dummy firmware images"
printf 'HSM-FIRMWARE-v1 :: simulated hsm core firmware payload\n' > images/hsm_fw.bin
printf 'BL2-BOOTLOADER-v1 :: simulated second-stage bootloader payload\n' > images/bl2.bin
printf 'APPLICATION-v1 :: simulated trusted application payload\n' > images/app.bin
echo "  wrote images/hsm_fw.bin, images/bl2.bin, images/app.bin"

# --- 4. sign images + manifests (version 1) ---------------------------
echo ""
echo "[setup] signing images and writing manifests (version 1)"

python3 tools/sign_image.py   images/hsm_fw.bin keys/hsm_signer_priv.pem images/hsm_fw.sig
python3 tools/make_manifest.py hsm_fw 1 images/hsm_fw.bin images/hsm_fw_manifest.json

python3 tools/sign_image.py   images/bl2.bin keys/oem_bl_priv.pem images/bl2.sig
python3 tools/make_manifest.py bl2 1 images/bl2.bin images/bl2_manifest.json

python3 tools/sign_image.py   images/app.bin keys/oem_app_priv.pem images/app.sig
python3 tools/make_manifest.py app 1 images/app.bin images/app_manifest.json

# --- 5. negative-test artifacts ---------------------------------------
echo ""
echo "[setup] creating negative-test artifacts (corrupted images)"
python3 tools/corrupt_image.py flip images/hsm_fw.bin images/hsm_fw_corrupt.bin
python3 tools/corrupt_image.py flip images/bl2.bin    images/bl2_corrupt.bin
python3 tools/corrupt_image.py flip images/app.bin    images/app_corrupt.bin

echo ""
echo "[setup] DONE. Next steps:"
echo "  make          # build the simulator"
echo "  ./run.sh      # run the happy-path boot + the failure demos"
echo "  make tests && ./run_tests.sh   # build and run unit tests"
