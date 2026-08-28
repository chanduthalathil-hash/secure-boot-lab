/*
 * tests/test_hsm_fw_verify.c
 *
 * Tests that ROM's verification of the HSM firmware:
 *   - PASSES for a correctly-signed image
 *   - FAILS for an image signed with the wrong (attacker) key
 *   - FAILS for a corrupted (bit-flipped) image
 *
 * These negative artifacts are produced by setup.sh / the tools before
 * this test runs. If they're missing, the test skips that case with a
 * note rather than failing spuriously.
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
    printf("== test_hsm_fw_verify ==\n");
    int all_ok = 1;
    unsigned long v = 0;

    /* Valid case: correct key, intact image. */
    int r_valid = verify_stage_image("TEST", "keys/rom_root_pub.pem",
                                     "images/hsm_fw.bin", "images/hsm_fw.sig",
                                     "images/hsm_fw_manifest.json", "hsm_fw", &v);
    all_ok &= expect("valid hsm_fw verifies", r_valid, 1);

    /* Negative: wrong key. Reuse the attacker public key against the
     * legitimately-signed image -- signature must fail. */
    if (access("keys/signing_test_keys/attacker_pub.pem", F_OK) == 0) {
        int r_wrongkey = verify_stage_image("TEST", "keys/signing_test_keys/attacker_pub.pem",
                                            "images/hsm_fw.bin", "images/hsm_fw.sig",
                                            "images/hsm_fw_manifest.json", "hsm_fw", &v);
        all_ok &= expect("wrong-key hsm_fw rejected", r_wrongkey, 0);
    } else {
        printf("  [SKIP] attacker_pub.pem missing\n");
    }

    /* Negative: corrupted image. */
    if (access("images/hsm_fw_corrupt.bin", F_OK) == 0) {
        int r_corrupt = verify_stage_image("TEST", "keys/rom_root_pub.pem",
                                           "images/hsm_fw_corrupt.bin", "images/hsm_fw.sig",
                                           "images/hsm_fw_manifest.json", "hsm_fw", &v);
        all_ok &= expect("corrupted hsm_fw rejected", r_corrupt, 0);
    } else {
        printf("  [SKIP] hsm_fw_corrupt.bin missing\n");
    }

    printf("== %s ==\n", all_ok ? "ALL PASS" : "FAILURES PRESENT");
    return all_ok ? 0 : 1;
}
