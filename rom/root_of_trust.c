/*
 * rom/root_of_trust.c
 *
 * On real hardware, rot_public_key_path() wouldn't exist -- the key (or
 * its hash) would be physically fused into the chip and read by the ROM
 * from a hardware register. This file is the simulator's stand-in for
 * that immutable anchor.
 */
#include <stdio.h>

#include "root_of_trust.h"
#include "../common/boot_algo.h"

const char *rot_public_key_path(void) {
    static char path[128];
    snprintf(path, sizeof(path), "keys/rom_root_pub%s.pem", boot_algo_suffix());
    return path;
}
