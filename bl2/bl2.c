/*
 * bl2/bl2.c
 *
 * Executing this means the HSM verified BL2. BL2 now verifies APP.
 */
#include <stdio.h>

#include "bl2.h"
#include "bl2_verify.h"
#include "../app/app_main.h"

int bl2_start(void) {
    printf("\n=== BL2 (running -- means HSM already verified it) ===\n");

    if (!bl2_verify_app("images/app.bin", "images/app.sig", "images/app_manifest.json")) {
        printf("[BL2] HALT: APP verification failed -- not handing off control\n");
        return 0;
    }

    printf("[BL2] APP verified. Handing off control to APP.\n");
    return app_main();
}
