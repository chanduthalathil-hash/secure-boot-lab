/*
 * bl2/bl2_verify.c
 */
#include <stdio.h>

#include "bl2_verify.h"
#include "../common/verify_common.h"
#include "../common/boot_algo.h"
#include "../hsm/hsm_rollback.h"

/* BL2 verifies APP with the OEM application public key. Note the
 * difference from the HSM stage: BL2 holds this public key directly
 * (public keys are not secret), whereas the HSM held ITS key behind an
 * API. Both are fine -- the HSM abstraction matters for private or
 * symmetric key material, which is not what a boot signature check uses.
 */
int bl2_verify_app(const char *image_path,
                   const char *sig_path,
                   const char *manifest_path) {
    unsigned long app_version = 0;

    char pubkey_path[128];
    snprintf(pubkey_path, sizeof(pubkey_path), "keys/oem_app_pub%s.pem", boot_algo_suffix());

    if (!verify_stage_image("BL2", pubkey_path, image_path, sig_path,
                            manifest_path, "app", &app_version)) {
        return 0;
    }

    if (!rollback_check_and_update("app", app_version)) {
        return 0;
    }

    return 1;
}
