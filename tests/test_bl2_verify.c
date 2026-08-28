/*
 * tests/test_bl2_verify.c
 *
 * Tests BL2 verification THROUGH the HSM key store API -- confirming
 * that the keystore returns only pass/fail and behaves correctly for
 * valid and invalid images.
 */
#include <stdio.h>
#include <unistd.h>

#include "../hsm/hsm_keystore.h"

static int expect(const char *name, int got, int want) {
    int ok = (got == want);
    printf("  [%s] %s (got=%d want=%d)\n", ok ? "PASS" : "FAIL", name, got, want);
    return ok;
}

int main(void) {
    printf("== test_bl2_verify ==\n");
    int all_ok = 1;
    unsigned long v = 0;

    /* Before init, the keystore must refuse to do anything. */
    int r_before = hsm_keystore_verify("bl2_verify_key", "images/bl2.bin",
                                       "images/bl2.sig", "images/bl2_manifest.json",
                                       "bl2", &v);
    all_ok &= expect("keystore refuses before init", r_before, 0);

    /* Initialize (models: HSM firmware verified, so keys unlock). */
    hsm_keystore_init();

    int r_valid = hsm_keystore_verify("bl2_verify_key", "images/bl2.bin",
                                      "images/bl2.sig", "images/bl2_manifest.json",
                                      "bl2", &v);
    all_ok &= expect("valid bl2 verifies via keystore", r_valid, 1);

    /* Unknown key label must be refused. */
    int r_badlabel = hsm_keystore_verify("no_such_key", "images/bl2.bin",
                                         "images/bl2.sig", "images/bl2_manifest.json",
                                         "bl2", &v);
    all_ok &= expect("unknown key label rejected", r_badlabel, 0);

    /* Corrupted BL2 must fail. */
    if (access("images/bl2_corrupt.bin", F_OK) == 0) {
        int r_corrupt = hsm_keystore_verify("bl2_verify_key", "images/bl2_corrupt.bin",
                                            "images/bl2.sig", "images/bl2_manifest.json",
                                            "bl2", &v);
        all_ok &= expect("corrupted bl2 rejected", r_corrupt, 0);
    } else {
        printf("  [SKIP] bl2_corrupt.bin missing\n");
    }

    printf("== %s ==\n", all_ok ? "ALL PASS" : "FAILURES PRESENT");
    return all_ok ? 0 : 1;
}
