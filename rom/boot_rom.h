/*
 * rom/boot_rom.h
 *
 * The immutable Boot ROM. This is where execution begins. It verifies
 * the HSM firmware using the root-of-trust key, and only on success
 * starts the HSM firmware.
 */
#ifndef BOOT_ROM_H
#define BOOT_ROM_H

/* The very first thing that runs. Returns the overall chain result. */
int boot_rom_main(void);

#endif /* BOOT_ROM_H */
