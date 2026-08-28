#!/usr/bin/env python3
"""
generate_keys.py

Generates all ECDSA P-256 keypairs needed for the secure boot lab.

Key hierarchy (see README.md for the full explanation):

  hsm_signer_priv.pem  <-> rom_root_pub.pem     : signs/verifies HSM firmware
  oem_bl_priv.pem      <-> oem_bl_pub.pem       : signs/verifies BL2
  oem_app_priv.pem     <-> oem_app_pub.pem      : signs/verifies APP

  signing_test_keys/attacker_priv.pem + attacker_pub.pem
      : an unrelated keypair used only in negative tests, to simulate
        someone signing an image with the WRONG key.

NOTE ON REALISM: on real silicon, the ROM's public key (or a hash of it)
lives in OTP fuses and the private counterpart is generated and used
offline, once, in a secure manufacturing facility -- it never touches
the device itself. Here, for simulation purposes, we generate both
halves on the same machine so you can inspect and experiment with them.
"""

import os
from cryptography.hazmat.primitives.asymmetric import ec
from cryptography.hazmat.primitives import serialization

KEYS_DIR = os.path.join(os.path.dirname(__file__), "..", "keys")
TEST_KEYS_DIR = os.path.join(KEYS_DIR, "signing_test_keys")


def make_keypair(priv_path, pub_path):
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
    os.makedirs(KEYS_DIR, exist_ok=True)
    os.makedirs(TEST_KEYS_DIR, exist_ok=True)

    print("[1/4] ROM <-> HSM firmware signing key")
    make_keypair(
        os.path.join(KEYS_DIR, "hsm_signer_priv.pem"),
        os.path.join(KEYS_DIR, "rom_root_pub.pem"),
    )

    print("[2/4] HSM <-> BL2 signing key")
    make_keypair(
        os.path.join(KEYS_DIR, "oem_bl_priv.pem"),
        os.path.join(KEYS_DIR, "oem_bl_pub.pem"),
    )

    print("[3/4] BL2 <-> APP signing key")
    make_keypair(
        os.path.join(KEYS_DIR, "oem_app_priv.pem"),
        os.path.join(KEYS_DIR, "oem_app_pub.pem"),
    )

    print("[4/4] Attacker test key (unrelated, for negative tests)")
    make_keypair(
        os.path.join(TEST_KEYS_DIR, "attacker_priv.pem"),
        os.path.join(TEST_KEYS_DIR, "attacker_pub.pem"),
    )

    print("\nAll keys generated.")


if __name__ == "__main__":
    main()
