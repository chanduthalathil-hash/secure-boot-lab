/*
 * tests/test_app_verify.c
 *
 * Tests BL2's verification of the APP image directly against the OEM
 * app public key.
 */
#include <stdio.h>
#include <unistd.h>

#include "../common/verify_common.h"

static int expect(const char *name, int got, int want) {
    int ok = (got == want);
    printf("  [%s] %s (got=%d want=%d)\n", ok ? "PASS" : "FAIL", name, got, want);
    return ok;
}

int main(void) {
    printf("== test_app_verify ==\n");
    int all_ok = 1;
    unsigned long v = 0;

    int r_valid = verify_stage_image("TEST", "keys/oem_app_pub.pem",
                                     "images/app.bin", "images/app.sig",
                                     "images/app_manifest.json", "app", &v);
    all_ok &= expect("valid app verifies", r_valid, 1);

    /* Wrong component name in expected check. */
    int r_wrongcomp = verify_stage_image("TEST", "keys/oem_app_pub.pem",
                                         "images/app.bin", "images/app.sig",
                                         "images/app_manifest.json", "bl2", &v);
    all_ok &= expect("wrong expected-component rejected", r_wrongcomp, 0);

    if (access("images/app_corrupt.bin", F_OK) == 0) {
        int r_corrupt = verify_stage_image("TEST", "keys/oem_app_pub.pem",
                                           "images/app_corrupt.bin", "images/app.sig",
                                           "images/app_manifest.json", "app", &v);
        all_ok &= expect("corrupted app rejected", r_corrupt, 0);
    } else {
        printf("  [SKIP] app_corrupt.bin missing\n");
    }

    printf("== %s ==\n", all_ok ? "ALL PASS" : "FAILURES PRESENT");
    return all_ok ? 0 : 1;
}
