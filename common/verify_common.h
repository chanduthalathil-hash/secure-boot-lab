/*
 * common/verify_common.h
 *
 * NOTE: this "common/" folder is a small, deliberate addition beyond the
 * original folder sketch. Each of ROM, HSM firmware, and BL2 need the
 * *same* three primitives -- hash a file, parse a manifest, verify an
 * ECDSA signature -- so this file exists once instead of being copied
 * three times. Real production firmware almost always factors this
 * out into a shared crypto/manifest library too, so this mirrors
 * reality rather than diverging from it. See README.md for the full
 * list of additions like this one.
 *
 * This is intentionally a MINIMAL manifest parser (flat key: value
 * JSON only, no nesting, no arrays) -- not a general JSON library.
 */
#ifndef VERIFY_COMMON_H
#define VERIFY_COMMON_H

#define MANIFEST_COMPONENT_MAX 64

typedef struct {
    char component[MANIFEST_COMPONENT_MAX];
    unsigned long version;
    unsigned long size;
    char sha256_hex[65]; /* 64 hex chars + null terminator */
} manifest_t;

/* Parses a flat JSON manifest file into `out`. Returns 1 on success. */
int manifest_parse(const char *manifest_path, manifest_t *out);

/*
 * Full verification of one boot stage image:
 *   1. parses the manifest
 *   2. checks manifest.component matches expected_component (if non-NULL)
 *   3. recomputes SHA-256 of the image file and compares to manifest.sha256
 *   4. verifies the signature in sig_path against the image bytes, using
 *      the public key in pubkey_pem_path
 *
 * Step 4 works for EITHER ECDSA-P256 or RSA-2048/PKCS#1 v1.5 signatures,
 * unmodified -- OpenSSL's EVP_DigestVerify* API dispatches on the type of
 * key it loads from pubkey_pem_path, not on anything this code decides.
 * Which algorithm's key/signature files get passed in is decided by the
 * caller via common/boot_algo.h, not by this function.
 *
 * On success, returns 1 and writes the manifest's version number into
 * *out_version (needed by the caller for the anti-rollback check).
 * On any failure, returns 0, and prints a human-readable reason to
 * stderr -- this lab is meant to be read, not just run.
 */
int verify_stage_image(const char *stage_label,
                        const char *pubkey_pem_path,
                        const char *image_path,
                        const char *sig_path,
                        const char *manifest_path,
                        const char *expected_component,
                        unsigned long *out_version);

#endif /* VERIFY_COMMON_H */
