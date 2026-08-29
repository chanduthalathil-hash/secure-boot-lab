/*
 * main.c
 *
 * Entry point for the secure boot chain simulator. Power-on starts the
 * Boot ROM; everything else follows from there.
 */
#include <stdio.h>

#include "rom/boot_rom.h"
#include "common/boot_algo.h"

int main(void) {
    printf("\n########################################################\n");
    printf("#  Secure Boot Chain Simulator                         #\n");
    printf("#  ROM -> HSM firmware -> BL2 -> APP                    #\n");
    printf("########################################################\n");
    printf("  signature algorithm: %s\n", boot_algo_name());
    printf("########################################################\n\n");

    int ok = boot_rom_main();

    printf("\n--------------------------------------------------------\n");
    if (ok) {
        printf("RESULT: BOOT SUCCESSFUL -- full chain of trust verified.\n");
    } else {
        printf("RESULT: BOOT HALTED -- chain of trust broken (see logs above).\n");
    }
    printf("--------------------------------------------------------\n\n");

    /* Non-zero exit code on failure so test scripts can detect it. */
    return ok ? 0 : 1;
}
