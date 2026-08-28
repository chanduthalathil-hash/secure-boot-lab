/*
 * hsm/hsm_rollback.c
 *
 * See hsm_rollback.h for the design notes.
 *
 * State file format (state/rollback_counters.txt), one line per
 * component:
 *     hsm_fw=1
 *     bl2=1
 *     app=1
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hsm_rollback.h"

#define STATE_FILE_PATH "state/rollback_counters.txt"
#define MAX_LINE 256
#define MAX_COMPONENTS 32

typedef struct {
    char component[64];
    unsigned long version;
} rollback_entry_t;

static int load_state(rollback_entry_t entries[MAX_COMPONENTS]) {
    FILE *f = fopen(STATE_FILE_PATH, "r");
    if (!f) return 0; /* no state file yet -- fresh lab, that's fine */

    int count = 0;
    char line[MAX_LINE];
    while (count < MAX_COMPONENTS && fgets(line, sizeof(line), f)) {
        char *eq = strchr(line, '=');
        if (!eq) continue;
        *eq = '\0';
        char *val = eq + 1;

        strncpy(entries[count].component, line, sizeof(entries[count].component) - 1);
        entries[count].component[sizeof(entries[count].component) - 1] = '\0';
        entries[count].version = strtoul(val, NULL, 10);
        count++;
    }
    fclose(f);
    return count;
}

static void save_state(rollback_entry_t entries[MAX_COMPONENTS], int count) {
    FILE *f = fopen(STATE_FILE_PATH, "w");
    if (!f) {
        fprintf(stderr, "  [rollback] WARNING: could not write state file %s\n", STATE_FILE_PATH);
        return;
    }
    for (int i = 0; i < count; i++) {
        fprintf(f, "%s=%lu\n", entries[i].component, entries[i].version);
    }
    fclose(f);
}

unsigned long rollback_get_last_version(const char *component) {
    rollback_entry_t entries[MAX_COMPONENTS];
    int count = load_state(entries);
    for (int i = 0; i < count; i++) {
        if (strcmp(entries[i].component, component) == 0) {
            return entries[i].version;
        }
    }
    return 0;
}

int rollback_check_and_update(const char *component, unsigned long candidate_version) {
    rollback_entry_t entries[MAX_COMPONENTS];
    int count = load_state(entries);

    int found_idx = -1;
    for (int i = 0; i < count; i++) {
        if (strcmp(entries[i].component, component) == 0) {
            found_idx = i;
            break;
        }
    }

    unsigned long last_known = (found_idx >= 0) ? entries[found_idx].version : 0;

    printf("  [rollback] component=%s candidate_version=%lu last_known_version=%lu\n",
           component, candidate_version, last_known);

    if (found_idx >= 0 && candidate_version < last_known) {
        printf("  [rollback] REJECTED: candidate version %lu is older than last"
               " known-good version %lu -- possible rollback attack\n",
               candidate_version, last_known);
        return 0;
    }

    if (found_idx >= 0) {
        entries[found_idx].version = candidate_version;
    } else {
        if (count < MAX_COMPONENTS) {
            strncpy(entries[count].component, component, sizeof(entries[count].component) - 1);
            entries[count].component[sizeof(entries[count].component) - 1] = '\0';
            entries[count].version = candidate_version;
            count++;
        }
    }

    save_state(entries, count);
    printf("  [rollback] ACCEPTED: version counter updated to %lu\n", candidate_version);
    return 1;
}
