/*
 * hsm/hsm_fw.c
 *
 * This code represents the firmware running ON the HSM core. The fact
 * that this function is executing AT ALL means ROM already verified its
 * signature -- that's the precondition modeled by ROM calling this only
 * on success. See README.md, section "The two-chain model".
 */
#include <stdio.h>

#include "hsm_fw.h"
#include "hsm_keystore.h"
#include "hsm_verify.h"
#include "../common/boot_algo.h"
#include "../bl2/bl2.h"

int hsm_fw_start(void) {
    printf("\n=== HSM FIRMWARE (running -- means ROM already verified it) ===\n");

    /* Only now, after our own firmware was verified, do we unlock keys. */
    if (!hsm_keystore_init()) {
        printf("[HSM-FW] FAIL: could not initialize key store\n");
        return 0;
    }

    char sig_path[128];
    snprintf(sig_path, sizeof(sig_path), "images/bl2%s.sig", boot_algo_suffix());

    /* Verify the next stage (BL2): signature via internal key + rollback. */
    if (!hsm_verify_bl2("images/bl2.bin", sig_path, "images/bl2_manifest.json")) {
        printf("[HSM-FW] HALT: BL2 verification failed -- not handing off control\n");
        return 0;
    }

    printf("[HSM-FW] BL2 verified. Handing off control to BL2.\n");

    /* Hand off to BL2 (in real hw: release/jump to the BL2 entry point). */
    return bl2_start();
}
