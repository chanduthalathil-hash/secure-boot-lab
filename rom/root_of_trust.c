/*
 * rom/root_of_trust.c
 *
 * On real hardware, rot_public_key_path() wouldn't exist -- the key (or
 * its hash) would be physically fused into the chip and read by the ROM
 * from a hardware register. This file is the simulator's stand-in for
 * that immutable anchor.
 */
#include "root_of_trust.h"

#define ROM_ROOT_PUBKEY_PATH "keys/rom_root_pub.pem"

const char *rot_public_key_path(void) {
    return ROM_ROOT_PUBKEY_PATH;
}
