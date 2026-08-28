/*
 * hsm/hsm_rollback.h
 *
 * Simulates the anti-rollback monotonic counters that real HSMs
 * typically implement in dedicated, write-once-per-boot-generation
 * hardware counters (or protected NVM). Here we persist last-known-good
 * versions in a plain state file at state/rollback_counters.txt --
 * NOT secure against tampering by design, since this is a software
 * simulator running as a normal user process. The point is to learn
 * the *logic* (reject any version older than the last accepted one),
 * not to reproduce the tamper-resistance itself.
 *
 * This module is physically stored under hsm/ because in a real system
 * anti-rollback counters are typically managed by the HSM, but the
 * functions here are called by ROM, HSM firmware, and BL2 alike --
 * every stage in this lab checks the NEXT stage's version before
 * accepting it.
 */
#ifndef HSM_ROLLBACK_H
#define HSM_ROLLBACK_H

/*
 * Returns 1 (allowed) if candidate_version >= last known-good version
 * for this component, and updates the stored counter to
 * candidate_version. Returns 0 (rejected) if candidate_version is
 * strictly less than the last known-good version -- this is the
 * rollback-attack case.
 *
 * First-ever check for a component (no prior record) always succeeds
 * and establishes the baseline.
 */
int rollback_check_and_update(const char *component, unsigned long candidate_version);

/* Returns the last known-good version for `component`, or 0 if none recorded yet. */
unsigned long rollback_get_last_version(const char *component);

#endif /* HSM_ROLLBACK_H */
