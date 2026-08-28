/*
 * bl2/bl2_verify.h
 *
 * BL2's helper to verify the APP: signature (with BL2's own OEM app
 * key, which it embeds -- unlike the HSM, BL2 is ordinary software and
 * holds a public verification key directly) plus anti-rollback.
 */
#ifndef BL2_VERIFY_H
#define BL2_VERIFY_H

int bl2_verify_app(const char *image_path,
                   const char *sig_path,
                   const char *manifest_path);

#endif /* BL2_VERIFY_H */
