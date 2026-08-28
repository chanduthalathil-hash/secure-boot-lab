/*
 * bl2/bl2.h
 *
 * Second-stage bootloader. Reached only after the HSM verified it.
 * BL2 in turn verifies the application (APP) before running it.
 */
#ifndef BL2_H
#define BL2_H

/* Runs BL2: verify + rollback-check APP, and if good, hand off to APP.
 * Returns 1 if the whole remaining chain succeeded, else 0. */
int bl2_start(void);

#endif /* BL2_H */
