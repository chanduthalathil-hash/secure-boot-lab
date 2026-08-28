#!/usr/bin/env python3
"""
make_manifest.py

Generates a manifest JSON file describing a firmware image: its
component name, version number (used for anti-rollback), size, and
SHA-256 hash.

The manifest format is DELIBERATELY flat and simple (no nested objects,
no arrays) because the C-side parser in this lab is a small
hand-written parser, not a general JSON library. Keep it this way if
you edit manifests by hand.

Usage:
    python3 make_manifest.py <component_name> <version> <image.bin> <output_manifest.json>

Example:
    python3 make_manifest.py hsm_fw 1 ../images/hsm_fw.bin ../images/hsm_fw_manifest.json
"""

import sys
import json
import hashlib


def main():
    if len(sys.argv) != 5:
        print(__doc__)
        sys.exit(1)

    component, version_str, image_path, manifest_out_path = sys.argv[1:5]
    version = int(version_str)

    with open(image_path, "rb") as f:
        image_bytes = f.read()

    digest_hex = hashlib.sha256(image_bytes).hexdigest()

    manifest = {
        "component": component,
        "version": version,
        "size": len(image_bytes),
        "sha256": digest_hex,
    }

    with open(manifest_out_path, "w") as f:
        json.dump(manifest, f, indent=2)
        f.write("\n")

    print(f"manifest written: {manifest_out_path}")
    print(json.dumps(manifest, indent=2))


if __name__ == "__main__":
    main()
