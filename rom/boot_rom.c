/*
 * rom/boot_rom.c
 *
 * The Boot ROM: the anchor of the whole chain. In this simulator it is
 * an ordinary function, but conceptually it is immutable code that runs
 * first and is trusted unconditionally.
 *
 * Its job here (your 5 core requirements, items 1-2):
 *   1. verify the HSM firmware's signature using the OTP/root key
 *   2. start the HSM firmware ONLY if that verification passed
 */
#include <stdio.h>

#include "boot_rom.h"
#include "root_of_trust.h"
#include "../common/verify_common.h"
#include "../hsm/hsm_rollback.h"
#include "../hsm/hsm_fw.h"

int boot_rom_main(void) {
    printf("=== BOOT ROM (immutable root of trust) ===\n");
    printf("[ROM] using root-of-trust key: %s\n", rot_public_key_path());

    unsigned long hsm_version = 0;

    /* Requirement 1: ROM verifies the HSM firmware. */
    if (!verify_stage_image("ROM", rot_public_key_path(),
                            "images/hsm_fw.bin", "images/hsm_fw.sig",
                            "images/hsm_fw_manifest.json", "hsm_fw", &hsm_version)) {
        printf("[ROM] HALT: HSM firmware verification failed -- HSM will NOT start.\n");
        printf("[ROM] Keys inside the HSM remain locked and unreachable.\n");
        return 0;
    }

    /* Anti-rollback also applies to the HSM firmware itself. */
    if (!rollback_check_and_update("hsm_fw", hsm_version)) {
        printf("[ROM] HALT: HSM firmware failed rollback check -- HSM will NOT start.\n");
        return 0;
    }

    printf("[ROM] HSM firmware verified. Starting HSM firmware.\n");

    /* Requirement 2: HSM initializes only because its firmware was valid. */
    return hsm_fw_start();
}
