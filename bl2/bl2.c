/*
 * bl2/bl2.c
 *
 * Executing this means the HSM verified BL2. BL2 now verifies APP.
 */
#include <stdio.h>

#include "bl2.h"
#include "bl2_verify.h"
#include "../common/boot_algo.h"
#include "../app/app_main.h"

int bl2_start(void) {
    printf("\n=== BL2 (running -- means HSM already verified it) ===\n");

    char sig_path[128];
    snprintf(sig_path, sizeof(sig_path), "images/app%s.sig", boot_algo_suffix());

    if (!bl2_verify_app("images/app.bin", sig_path, "images/app_manifest.json")) {
        printf("[BL2] HALT: APP verification failed -- not handing off control\n");
        return 0;
    }

    printf("[BL2] APP verified. Handing off control to APP.\n");
    return app_main();
}
