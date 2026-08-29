#!/usr/bin/env python3
"""
generate_keys.py

Generates all keypairs needed for the secure boot lab, for one signature
algorithm at a time.

Usage:
    python3 generate_keys.py            # ECDSA P-256 (default), no suffix
    python3 generate_keys.py --algo rsa # RSA-2048, files get a _rsa suffix

Key hierarchy (see README.md for the full explanation):

  hsm_signer_priv<sfx>.pem  <-> rom_root_pub<sfx>.pem   : signs/verifies HSM firmware
  oem_bl_priv<sfx>.pem      <-> oem_bl_pub<sfx>.pem     : signs/verifies BL2
  oem_app_priv<sfx>.pem     <-> oem_app_pub<sfx>.pem    : signs/verifies APP

  signing_test_keys/attacker_priv<sfx>.pem + attacker_pub<sfx>.pem
      : an unrelated keypair used only in negative tests, to simulate
        someone signing an image with the WRONG key.

Both algorithms are real, independent key hierarchies -- running this
script for both --algo values (as setup.sh does) produces two complete,
unrelated sets of keys, coexisting side by side via the filename suffix.
sign_image.py auto-detects which algorithm a given private key uses, so
signing itself needs no --algo flag.

NOTE ON REALISM: on real silicon, the ROM's public key (or a hash of it)
lives in OTP fuses and the private counterpart is generated and used
offline, once, in a secure manufacturing facility -- it never touches
the device itself. Here, for simulation purposes, we generate both
halves on the same machine so you can inspect and experiment with them.
"""

import argparse
import os
from cryptography.hazmat.primitives.asymmetric import ec, rsa
from cryptography.hazmat.primitives import serialization

KEYS_DIR = os.path.join(os.path.dirname(__file__), "..", "keys")
TEST_KEYS_DIR = os.path.join(KEYS_DIR, "signing_test_keys")


def make_keypair(priv_path, pub_path, algo):
    if algo == "rsa":
        private_key = rsa.generate_private_key(public_exponent=65537, key_size=2048)
    else:
        private_key = ec.generate_private_key(ec.SECP256R1())
    public_key = private_key.public_key()

    priv_pem = private_key.private_bytes(
        encoding=serialization.Encoding.PEM,
        format=serialization.PrivateFormat.PKCS8,
        encryption_algorithm=serialization.NoEncryption(),
    )
    pub_pem = public_key.public_bytes(
        encoding=serialization.Encoding.PEM,
        format=serialization.PublicFormat.SubjectPublicKeyInfo,
    )

    with open(priv_path, "wb") as f:
        f.write(priv_pem)
    with open(pub_path, "wb") as f:
        f.write(pub_pem)

    print(f"  generated {os.path.relpath(priv_path)}")
    print(f"  generated {os.path.relpath(pub_path)}")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--algo", choices=["ecdsa", "rsa"], default="ecdsa")
    args = parser.parse_args()
    sfx = "" if args.algo == "ecdsa" else "_rsa"
    label = "ECDSA P-256" if args.algo == "ecdsa" else "RSA-2048"

    os.makedirs(KEYS_DIR, exist_ok=True)
    os.makedirs(TEST_KEYS_DIR, exist_ok=True)

    print(f"[1/4] ROM <-> HSM firmware signing key ({label})")
    make_keypair(
        os.path.join(KEYS_DIR, f"hsm_signer_priv{sfx}.pem"),
        os.path.join(KEYS_DIR, f"rom_root_pub{sfx}.pem"),
        args.algo,
    )

    print(f"[2/4] HSM <-> BL2 signing key ({label})")
    make_keypair(
        os.path.join(KEYS_DIR, f"oem_bl_priv{sfx}.pem"),
        os.path.join(KEYS_DIR, f"oem_bl_pub{sfx}.pem"),
        args.algo,
    )

    print(f"[3/4] BL2 <-> APP signing key ({label})")
    make_keypair(
        os.path.join(KEYS_DIR, f"oem_app_priv{sfx}.pem"),
        os.path.join(KEYS_DIR, f"oem_app_pub{sfx}.pem"),
        args.algo,
    )

    print(f"[4/4] Attacker test key ({label}, unrelated, for negative tests)")
    make_keypair(
        os.path.join(TEST_KEYS_DIR, f"attacker_priv{sfx}.pem"),
        os.path.join(TEST_KEYS_DIR, f"attacker_pub{sfx}.pem"),
        args.algo,
    )

    print(f"\nAll {label} keys generated.")


if __name__ == "__main__":
    main()
