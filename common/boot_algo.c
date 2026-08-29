/*
 * common/boot_algo.c
 *
 * See boot_algo.h. Read once (lazily) and cached -- the algorithm
 * choice is fixed for the life of one run.
 */
#include <stdlib.h>
#include <string.h>

#include "boot_algo.h"

static int resolved = 0;
static int is_rsa = 0;

static void resolve(void) {
    const char *env = getenv("SECURE_BOOT_ALGO");
    is_rsa = (env != NULL && strcmp(env, "rsa") == 0);
    resolved = 1;
}

const char *boot_algo_suffix(void) {
    if (!resolved) resolve();
    return is_rsa ? "_rsa" : "";
}

const char *boot_algo_name(void) {
    if (!resolved) resolve();
    return is_rsa ? "RSA-2048 / PKCS#1 v1.5 (SHA-256)" : "ECDSA P-256 (SHA-256)";
}
