# Makefile for the secure boot chain simulator
#
# Targets:
#   make            - build the main simulator into bin/secure_boot_sim
#   make tests      - build the test binaries into bin/
#   make clean      - remove build artifacts
#
# Requires: gcc, OpenSSL dev headers (libssl-dev on Ubuntu/WSL2).

CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -I.
LDFLAGS = -lssl -lcrypto

BIN_DIR = bin

# Shared sources used by the main simulator
COMMON_SRCS = \
	common/verify_common.c \
	hsm/hsm_rollback.c \
	hsm/hsm_keystore.c \
	hsm/hsm_verify.c \
	hsm/hsm_fw.c \
	bl2/bl2.c \
	bl2/bl2_verify.c \
	app/app_main.c \
	rom/root_of_trust.c \
	rom/boot_rom.c

SIM_SRCS = main.c $(COMMON_SRCS)

.PHONY: all tests clean

all: $(BIN_DIR)/secure_boot_sim

$(BIN_DIR)/secure_boot_sim: $(SIM_SRCS) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $(SIM_SRCS) $(LDFLAGS)
	@echo "built $@"

# --- tests ---
# Each test reuses the common sources (minus main.c / boot_rom entry) and
# supplies its own main().

tests: \
	$(BIN_DIR)/test_hsm_fw_verify \
	$(BIN_DIR)/test_bl2_verify \
	$(BIN_DIR)/test_app_verify \
	$(BIN_DIR)/test_rollback

TEST_SUPPORT = \
	common/verify_common.c \
	hsm/hsm_rollback.c \
	hsm/hsm_keystore.c

$(BIN_DIR)/test_hsm_fw_verify: tests/test_hsm_fw_verify.c $(TEST_SUPPORT) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(BIN_DIR)/test_bl2_verify: tests/test_bl2_verify.c $(TEST_SUPPORT) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(BIN_DIR)/test_app_verify: tests/test_app_verify.c $(TEST_SUPPORT) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(BIN_DIR)/test_rollback: tests/test_rollback.c hsm/hsm_rollback.c | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

clean:
	rm -rf $(BIN_DIR)
	@echo "cleaned build artifacts"
