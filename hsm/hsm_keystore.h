/*
 * hsm/hsm_keystore.h
 *
 * A deliberately simple stand-in for a real HSM key store. The point
 * this module demonstrates -- and the ONE thing to take away from it --
 * is: callers never get a raw key back. They get a boolean answer to
 * "does this pass?", produced by a function that internally uses the
 * key. This is the same shape as a real client-server HSM API
 * (see wolfHSM): the client never receives key material, only a
 * result.
 *
 * This lab's keystore is intentionally minimal (it just wraps the
 * public-key verification already implemented in common/). Stage 2
 * of your learning plan (HSM service model) is where this gets
 * expanded into a real client/server boundary with its own process
 * separation -- don't over-build it here.
 */
#ifndef HSM_KEYSTORE_H
#define HSM_KEYSTORE_H

/* Must be called once, only after the HSM firmware's OWN signature has
 * been verified by ROM. Loads the keys this HSM instance is allowed to
 * use. Returns 1 on success. */
int hsm_keystore_init(void);

/*
 * Verifies BL2 using the key held inside the HSM key store. The
 * caller supplies paths to the image/signature/manifest; the actual
 * public key used is looked up internally by key_label and never
 * handed back to the caller.
 *
 * Returns 1 (valid) or 0 (invalid). On success, *out_version is set
 * to the version declared in BL2's manifest.
 */
int hsm_keystore_verify(const char *key_label,
                         const char *image_path,
                         const char *sig_path,
                         const char *manifest_path,
                         const char *expected_component,
                         unsigned long *out_version);

#endif /* HSM_KEYSTORE_H */
