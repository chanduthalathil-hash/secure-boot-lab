/*
 * hsm/hsm_verify.h
 *
 * Thin wrapper the HSM firmware uses to verify the NEXT stage (BL2).
 * It combines the keystore-based signature check with the
 * anti-rollback check, so the HSM firmware body (hsm_fw.c) reads
 * cleanly.
 */
#ifndef HSM_VERIFY_H
#define HSM_VERIFY_H

/* Verifies BL2's signature (via the internal key store) AND its
 * anti-rollback version. Returns 1 only if BOTH pass. */
int hsm_verify_bl2(const char *image_path,
                   const char *sig_path,
                   const char *manifest_path);

#endif /* HSM_VERIFY_H */
