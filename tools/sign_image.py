#!/usr/bin/env python3
"""
sign_image.py

Computes the SHA-256 hash of a firmware image and signs the raw image
bytes, writing the signature to a .sig file. Works for either an ECDSA
P-256 or an RSA-2048 private key -- the key TYPE is detected from the
PEM file itself, so no --algo flag is needed; whichever key you point
it at determines the signature scheme.

Usage:
    python3 sign_image.py <image.bin> <signing_priv_key.pem> <output.sig>

Example:
    python3 sign_image.py ../images/hsm_fw.bin ../keys/hsm_signer_priv.pem ../images/hsm_fw.sig
    python3 sign_image.py ../images/hsm_fw.bin ../keys/hsm_signer_priv_rsa.pem ../images/hsm_fw_rsa.sig
"""

import sys
import hashlib
from cryptography.hazmat.primitives import hashes, serialization
from cryptography.hazmat.primitives.asymmetric import ec, rsa, padding


def main():
    if len(sys.argv) != 4:
        print(__doc__)
        sys.exit(1)

    image_path, priv_key_path, sig_out_path = sys.argv[1:4]

    with open(image_path, "rb") as f:
        image_bytes = f.read()

    # Print the digest for human inspection / manifest cross-check, but
    # sign the RAW image bytes below. Passing a Signature algorithm object
    # (ec.ECDSA(SHA256) / SHA256() for RSA) makes the library hash the
    # input once internally -- this must match the C verifier, which uses
    # EVP_DigestVerify over the raw image bytes. (If we signed the digest
    # here, it would get hashed a second time and never verify.)
    digest = hashlib.sha256(image_bytes).digest()
    print(f"image:       {image_path}")
    print(f"size:        {len(image_bytes)} bytes")
    print(f"sha256:      {digest.hex()}")

    with open(priv_key_path, "rb") as f:
        private_key = serialization.load_pem_private_key(f.read(), password=None)

    if isinstance(private_key, rsa.RSAPrivateKey):
        # PKCS#1 v1.5 -- OpenSSL's default RSA verify padding, matching
        # what the C verifier's EVP_DigestVerify* will assume.
        signature = private_key.sign(image_bytes, padding.PKCS1v15(), hashes.SHA256())
        algo_label = f"RSA-{private_key.key_size} / PKCS#1 v1.5"
    elif isinstance(private_key, ec.EllipticCurvePrivateKey):
        signature = private_key.sign(image_bytes, ec.ECDSA(hashes.SHA256()))
        algo_label = "ECDSA (DER)"
    else:
        print(f"ERROR: unsupported private key type: {type(private_key).__name__}")
        sys.exit(1)

    with open(sig_out_path, "wb") as f:
        f.write(signature)

    print(f"signature:   {sig_out_path} ({len(signature)} bytes, {algo_label})")
    print(f"signed with: {priv_key_path}")


if __name__ == "__main__":
    main()
