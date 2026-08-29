/*
 * common/boot_algo.h
 *
 * Selects which signature algorithm's key/signature files the chain
 * uses this run: ECDSA P-256 (the default) or RSA-2048/PKCS#1 v1.5.
 * Both are fully real -- setup.sh generates a genuine key hierarchy and
 * genuine signatures for each; verify_stage_image() itself never
 * branches on algorithm (see verify_common.h) because OpenSSL's EVP
 * API already dispatches on the loaded key's type. This module only
 * decides which *files* get loaded.
 *
 * Selected via the SECURE_BOOT_ALGO environment variable:
 *   unset or "ecdsa"  -> keys/NAME.pem, images/NAME.sig       (default)
 *   "rsa"              -> keys/NAME_rsa.pem, images/NAME_rsa.sig
 *
 * Example:
 *   SECURE_BOOT_ALGO=rsa ./bin/secure_boot_sim
 */
#ifndef BOOT_ALGO_H
#define BOOT_ALGO_H

/* "" for ECDSA (default), "_rsa" for RSA -- append directly to the base
 * key/signature filename, e.g. "keys/rom_root_pub" + suffix + ".pem". */
const char *boot_algo_suffix(void);

/* Human-readable name of the active algorithm, for banners/logs. */
const char *boot_algo_name(void);

#endif /* BOOT_ALGO_H */
