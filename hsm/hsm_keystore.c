/*
 * hsm/hsm_keystore.c
 *
 * See hsm_keystore.h for the design intent. The key insight this
 * models: the caller passes a key_label (a name), never a key. The
 * mapping from label -> actual public key file lives inside here, and
 * only a pass/fail boolean crosses back out.
 */
#include <stdio.h>
#include <string.h>

#include "hsm_keystore.h"
#include "../common/verify_common.h"
#include "../common/boot_algo.h"

/* label -> public key file, resolved internally and never returned.
 * pubkey_path_fmt takes one %s: the active algorithm's suffix from
 * boot_algo_suffix() ("" for ECDSA, "_rsa" for RSA). */
typedef struct {
    const char *label;
    const char *pubkey_path_fmt;
} key_entry_t;

static const key_entry_t g_keys[] = {
    { "bl2_verify_key", "keys/oem_bl_pub%s.pem" },
};

static int g_keystore_ready = 0;

int hsm_keystore_init(void) {
    /* In real hardware this is where the HSM would decrypt/unwrap its
     * key material using a hardware-unique key, only reachable because
     * the HSM firmware itself was already verified by ROM. Here we just
     * mark the store as usable. */
    g_keystore_ready = 1;
    printf("  [hsm-keystore] key store unlocked (%zu key(s) available, held internally)\n",
           sizeof(g_keys) / sizeof(g_keys[0]));
    return 1;
}

static const char *resolve_key(const char *label) {
    static char path[128];
    for (size_t i = 0; i < sizeof(g_keys) / sizeof(g_keys[0]); i++) {
        if (strcmp(g_keys[i].label, label) == 0) {
            snprintf(path, sizeof(path), g_keys[i].pubkey_path_fmt, boot_algo_suffix());
            return path;
        }
    }
    return NULL;
}

int hsm_keystore_verify(const char *key_label,
                         const char *image_path,
                         const char *sig_path,
                         const char *manifest_path,
                         const char *expected_component,
                         unsigned long *out_version) {
    if (!g_keystore_ready) {
        fprintf(stderr, "  [hsm-keystore] REFUSED: key store not initialized"
                        " (HSM firmware not verified?)\n");
        return 0;
    }

    const char *pubkey_path = resolve_key(key_label);
    if (!pubkey_path) {
        fprintf(stderr, "  [hsm-keystore] REFUSED: no key with label '%s'\n", key_label);
        return 0;
    }

    printf("  [hsm-keystore] using internal key '%s' to verify (key bytes never leave HSM)\n",
           key_label);

    /* The caller gets back only this integer result, never the key. */
    return verify_stage_image("HSM-FW", pubkey_path, image_path, sig_path,
                              manifest_path, expected_component, out_version);
}
