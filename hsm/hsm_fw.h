/*
 * hsm/hsm_fw.h
 *
 * The HSM firmware's "entry point". ROM calls hsm_fw_start() ONLY
 * after it has verified this firmware's own signature. Inside,
 * hsm_fw_start() initializes the key store and then verifies BL2.
 */
#ifndef HSM_FW_H
#define HSM_FW_H

/* Runs the HSM firmware: unlock key store, verify + rollback-check BL2,
 * and if BL2 is good, hand control to it. Returns the final chain
 * result (1 = whole chain succeeded, 0 = failed somewhere below). */
int hsm_fw_start(void);

#endif /* HSM_FW_H */
