/*
 * app/app_main.c
 *
 * The trusted application. If this prints, the full chain of trust
 * ROM -> HSM firmware -> BL2 -> APP held end to end.
 */
#include <stdio.h>

#include "app_main.h"

int app_main(void) {
    printf("\n=== APPLICATION (running -- means BL2 already verified it) ===\n");
    printf("[APP] Hello from the verified application!\n");
    printf("[APP] Full chain of trust held: ROM -> HSM firmware -> BL2 -> APP\n");
    return 1;
}
