#!/usr/bin/env python3
"""
sign_image.py

Computes the SHA-256 hash of a firmware image and signs that hash with
an ECDSA P-256 private key, writing the raw DER signature bytes to a
.sig file.

Usage:
    python3 sign_image.py <image.bin> <signing_priv_key.pem> <output.sig>

Example:
    python3 sign_image.py ../images/hsm_fw.bin ../keys/hsm_signer_priv.pem ../images/hsm_fw.sig
"""

import sys
import hashlib
from cryptography.hazmat.primitives import hashes, serialization
from cryptography.hazmat.primitives.asymmetric import ec


def main():
    if len(sys.argv) != 4:
        print(__doc__)
        sys.exit(1)

    image_path, priv_key_path, sig_out_path = sys.argv[1:4]

    with open(image_path, "rb") as f:
        image_bytes = f.read()

    # Print the digest for human inspection / manifest cross-check, but
    # sign the RAW image bytes below. Passing ec.ECDSA(SHA256) makes the
    # library hash the input once internally -- this must match the C
    # verifier, which uses EVP_DigestVerify over the raw image bytes.
    # (If we signed the digest here, it would get hashed a second time
    # and never verify.)
    digest = hashlib.sha256(image_bytes).digest()
    print(f"image:       {image_path}")
    print(f"size:        {len(image_bytes)} bytes")
    print(f"sha256:      {digest.hex()}")

    with open(priv_key_path, "rb") as f:
        private_key = serialization.load_pem_private_key(f.read(), password=None)

    signature = private_key.sign(image_bytes, ec.ECDSA(hashes.SHA256()))

    with open(sig_out_path, "wb") as f:
        f.write(signature)

    print(f"signature:   {sig_out_path} ({len(signature)} bytes, DER-encoded ECDSA)")
    print(f"signed with: {priv_key_path}")


if __name__ == "__main__":
    main()
