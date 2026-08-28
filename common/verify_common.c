/*
 * common/verify_common.c
 *
 * See verify_common.h for why this shared module exists.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <openssl/evp.h>
#include <openssl/pem.h>

#include "verify_common.h"

/* ---------- tiny flat-JSON field extraction ---------- */

static const char *find_key(const char *json, const char *key) {
    char pattern[128];
    snprintf(pattern, sizeof(pattern), "\"%s\"", key);

    const char *p = strstr(json, pattern);
    if (!p) return NULL;
    p += strlen(pattern);

    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') p++;
    if (*p != ':') return NULL;
    p++;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') p++;
    return p;
}

static int json_get_string(const char *json, const char *key, char *out, size_t out_size) {
    const char *p = find_key(json, key);
    if (!p || *p != '"') return 0;
    p++;
    size_t i = 0;
    while (*p && *p != '"' && i < out_size - 1) {
        out[i++] = *p++;
    }
    out[i] = '\0';
    return 1;
}

static int json_get_ulong(const char *json, const char *key, unsigned long *out) {
    const char *p = find_key(json, key);
    if (!p) return 0;
    char *endptr = NULL;
    unsigned long v = strtoul(p, &endptr, 10);
    if (endptr == p) return 0;
    *out = v;
    return 1;
}

int manifest_parse(const char *manifest_path, manifest_t *out) {
    FILE *f = fopen(manifest_path, "rb");
    if (!f) {
        fprintf(stderr, "  [manifest] cannot open %s\n", manifest_path);
        return 0;
    }

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *buf = malloc((size_t)fsize + 1);
    if (!buf) {
        fclose(f);
        return 0;
    }
    size_t read_n = fread(buf, 1, (size_t)fsize, f);
    fclose(f);
    buf[read_n] = '\0';

    memset(out, 0, sizeof(*out));
    int ok = 1;
    ok &= json_get_string(buf, "component", out->component, sizeof(out->component));
    ok &= json_get_ulong(buf, "version", &out->version);
    ok &= json_get_ulong(buf, "size", &out->size);
    ok &= json_get_string(buf, "sha256", out->sha256_hex, sizeof(out->sha256_hex));

    free(buf);

    if (!ok) {
        fprintf(stderr, "  [manifest] failed to parse required fields from %s\n", manifest_path);
    }
    return ok;
}

/* ---------- hashing ---------- */

static int sha256_file(const char *path, unsigned char out[32], long *out_size) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        fprintf(stderr, "  [hash] cannot open image %s\n", path);
        return 0;
    }

    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(ctx, EVP_sha256(), NULL);

    unsigned char buf[4096];
    size_t n;
    long total = 0;
    while ((n = fread(buf, 1, sizeof(buf), f)) > 0) {
        EVP_DigestUpdate(ctx, buf, n);
        total += (long)n;
    }
    fclose(f);

    unsigned int len = 0;
    EVP_DigestFinal_ex(ctx, out, &len);
    EVP_MD_CTX_free(ctx);

    if (out_size) *out_size = total;
    return (len == 32);
}

static void bytes_to_hex(const unsigned char *bytes, size_t len, char *out_hex) {
    static const char hexchars[] = "0123456789abcdef";
    for (size_t i = 0; i < len; i++) {
        out_hex[i * 2]     = hexchars[(bytes[i] >> 4) & 0xF];
        out_hex[i * 2 + 1] = hexchars[bytes[i] & 0xF];
    }
    out_hex[len * 2] = '\0';
}

/* ---------- signature verification ---------- */

static int verify_ecdsa_signature_over_file(const char *pubkey_pem_path,
                                             const char *image_path,
                                             const char *sig_path) {
    FILE *pf = fopen(pubkey_pem_path, "r");
    if (!pf) {
        fprintf(stderr, "  [sig] cannot open public key %s\n", pubkey_pem_path);
        return 0;
    }
    EVP_PKEY *pkey = PEM_read_PUBKEY(pf, NULL, NULL, NULL);
    fclose(pf);
    if (!pkey) {
        fprintf(stderr, "  [sig] failed to parse public key %s\n", pubkey_pem_path);
        return 0;
    }

    FILE *sf = fopen(sig_path, "rb");
    if (!sf) {
        fprintf(stderr, "  [sig] cannot open signature %s\n", sig_path);
        EVP_PKEY_free(pkey);
        return 0;
    }
    unsigned char sig[512];
    size_t sig_len = fread(sig, 1, sizeof(sig), sf);
    fclose(sf);

    FILE *imf = fopen(image_path, "rb");
    if (!imf) {
        fprintf(stderr, "  [sig] cannot open image %s\n", image_path);
        EVP_PKEY_free(pkey);
        return 0;
    }
    fseek(imf, 0, SEEK_END);
    long fsize = ftell(imf);
    fseek(imf, 0, SEEK_SET);
    unsigned char *image_buf = malloc((size_t)fsize);
    size_t got = fread(image_buf, 1, (size_t)fsize, imf);
    fclose(imf);
    (void)got;

    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    int ok = 0;
    if (EVP_DigestVerifyInit(mdctx, NULL, EVP_sha256(), NULL, pkey) == 1) {
        if (EVP_DigestVerifyUpdate(mdctx, image_buf, (size_t)fsize) == 1) {
            int rc = EVP_DigestVerifyFinal(mdctx, sig, sig_len);
            ok = (rc == 1);
        }
    }

    EVP_MD_CTX_free(mdctx);
    free(image_buf);
    EVP_PKEY_free(pkey);
    return ok;
}

/* ---------- orchestrator ---------- */

int verify_stage_image(const char *stage_label,
                        const char *pubkey_pem_path,
                        const char *image_path,
                        const char *sig_path,
                        const char *manifest_path,
                        const char *expected_component,
                        unsigned long *out_version) {
    printf("[%s] verifying '%s' ...\n", stage_label, image_path);

    manifest_t m;
    if (!manifest_parse(manifest_path, &m)) {
        printf("[%s] FAIL: could not parse manifest %s\n", stage_label, manifest_path);
        return 0;
    }

    if (expected_component && strcmp(m.component, expected_component) != 0) {
        printf("[%s] FAIL: manifest claims component '%s', expected '%s'\n",
               stage_label, m.component, expected_component);
        return 0;
    }

    unsigned char digest[32];
    long file_size = 0;
    if (!sha256_file(image_path, digest, &file_size)) {
        printf("[%s] FAIL: could not hash image %s\n", stage_label, image_path);
        return 0;
    }

    char computed_hex[65];
    bytes_to_hex(digest, 32, computed_hex);

    printf("[%s] manifest version : %lu\n", stage_label, m.version);
    printf("[%s] manifest sha256   : %s\n", stage_label, m.sha256_hex);
    printf("[%s] computed sha256   : %s\n", stage_label, computed_hex);

    if (strcmp(computed_hex, m.sha256_hex) != 0) {
        printf("[%s] FAIL: image hash does not match manifest -- image is corrupted"
               " or manifest does not belong to this image\n", stage_label);
        return 0;
    }

    if ((unsigned long)file_size != m.size) {
        printf("[%s] FAIL: image size %ld does not match manifest size %lu\n",
               stage_label, file_size, m.size);
        return 0;
    }

    if (!verify_ecdsa_signature_over_file(pubkey_pem_path, image_path, sig_path)) {
        printf("[%s] FAIL: signature verification failed -- wrong key, wrong"
               " signature, or tampered image\n", stage_label);
        return 0;
    }

    printf("[%s] PASS: hash matches, signature valid\n", stage_label);

    if (out_version) *out_version = m.version;
    return 1;
}
