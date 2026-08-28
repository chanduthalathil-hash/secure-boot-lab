/*
 * hsm/hsm_verify.c
 */
#include <stdio.h>

#include "hsm_verify.h"
#include "hsm_keystore.h"
#include "hsm_rollback.h"

int hsm_verify_bl2(const char *image_path,
                   const char *sig_path,
                   const char *manifest_path) {
    unsigned long bl2_version = 0;

    /* Step 1: cryptographic verification using the HSM-held key. */
    if (!hsm_keystore_verify("bl2_verify_key", image_path, sig_path,
                             manifest_path, "bl2", &bl2_version)) {
        return 0;
    }

    /* Step 2: anti-rollback. Even a perfectly-signed old image must be
     * rejected if it's a downgrade. */
    if (!rollback_check_and_update("bl2", bl2_version)) {
        return 0;
    }

    return 1;
}
