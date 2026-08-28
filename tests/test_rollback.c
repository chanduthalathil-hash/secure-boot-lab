/*
 * tests/test_rollback.c
 *
 * Tests the anti-rollback logic in isolation, using a throwaway
 * component name so it doesn't disturb the real chain's state.
 *
 * IMPORTANT: this test writes to state/rollback_counters.txt via the
 * same module the chain uses. It uses a component name ("_rollback_test")
 * that the real chain never uses, so it won't interfere -- but it does
 * leave a harmless extra line in the state file.
 */
#include <stdio.h>

#include "../hsm/hsm_rollback.h"

static int expect(const char *name, int got, int want) {
    int ok = (got == want);
    printf("  [%s] %s (got=%d want=%d)\n", ok ? "PASS" : "FAIL", name, got, want);
    return ok;
}

int main(void) {
    printf("== test_rollback ==\n");
    int all_ok = 1;
    const char *comp = "_rollback_test";

    /* First acceptance establishes a baseline. Use a high number so
     * re-running the test is deterministic regardless of prior state. */
    all_ok &= expect("accept v100 (baseline)", rollback_check_and_update(comp, 100), 1);

    /* Same version is allowed (>=). */
    all_ok &= expect("accept v100 again (equal)", rollback_check_and_update(comp, 100), 1);

    /* Newer version is allowed. */
    all_ok &= expect("accept v101 (newer)", rollback_check_and_update(comp, 101), 1);

    /* Older version is rejected -- this is the rollback attack. */
    all_ok &= expect("reject v50 (older)", rollback_check_and_update(comp, 50), 0);

    /* After rejection, the counter must NOT have moved backwards. */
    all_ok &= expect("counter still at 101", (int)rollback_get_last_version(comp), 101);

    printf("== %s ==\n", all_ok ? "ALL PASS" : "FAILURES PRESENT");
    return all_ok ? 0 : 1;
}
