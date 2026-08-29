/*
 * tests/test_rsa_verify.c
 *
 * Proves the RSA-2048/PKCS#1 v1.5 key/signature set setup.sh generates
 * actually verifies through the real C verifier (verify_stage_image()),
 * not just in the browser console. Mirrors the coverage the ECDSA tests
 * have across hsm_fw/bl2/app, combined into one file since it's all the
 * same call with different _rsa-suffixed fixture paths.
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
    printf("== test_rsa_verify ==\n");
    int all_ok = 1;
    unsigned long v = 0;

    /* Valid case for each real stage, verified with the RSA key set. */
    int r_hsm = verify_stage_image("TEST", "keys/rom_root_pub_rsa.pem",
                                   "images/hsm_fw.bin", "images/hsm_fw_rsa.sig",
                                   "images/hsm_fw_manifest.json", "hsm_fw", &v);
    all_ok &= expect("valid hsm_fw verifies (RSA)", r_hsm, 1);

    int r_bl2 = verify_stage_image("TEST", "keys/oem_bl_pub_rsa.pem",
                                   "images/bl2.bin", "images/bl2_rsa.sig",
                                   "images/bl2_manifest.json", "bl2", &v);
    all_ok &= expect("valid bl2 verifies (RSA)", r_bl2, 1);

    int r_app = verify_stage_image("TEST", "keys/oem_app_pub_rsa.pem",
                                   "images/app.bin", "images/app_rsa.sig",
                                   "images/app_manifest.json", "app", &v);
    all_ok &= expect("valid app verifies (RSA)", r_app, 1);

    /* Negative: wrong RSA key. */
    if (access("keys/signing_test_keys/attacker_pub_rsa.pem", F_OK) == 0) {
        int r_wrongkey = verify_stage_image("TEST", "keys/signing_test_keys/attacker_pub_rsa.pem",
                                            "images/hsm_fw.bin", "images/hsm_fw_rsa.sig",
                                            "images/hsm_fw_manifest.json", "hsm_fw", &v);
        all_ok &= expect("wrong-key hsm_fw rejected (RSA)", r_wrongkey, 0);
    } else {
        printf("  [SKIP] attacker_pub_rsa.pem missing\n");
    }

    /* Negative: corrupted image, real RSA signature unchanged. */
    if (access("images/hsm_fw_corrupt.bin", F_OK) == 0) {
        int r_corrupt = verify_stage_image("TEST", "keys/rom_root_pub_rsa.pem",
                                           "images/hsm_fw_corrupt.bin", "images/hsm_fw_rsa.sig",
                                           "images/hsm_fw_manifest.json", "hsm_fw", &v);
        all_ok &= expect("corrupted hsm_fw rejected (RSA)", r_corrupt, 0);
    } else {
        printf("  [SKIP] hsm_fw_corrupt.bin missing\n");
    }

    printf("== %s ==\n", all_ok ? "ALL PASS" : "FAILURES PRESENT");
    return all_ok ? 0 : 1;
}
