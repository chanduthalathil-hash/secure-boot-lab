/*
 * rom/root_of_trust.h
 *
 * Models the hardware root of trust: the ROM public key that on real
 * silicon lives in OTP fuses. Here it's just the path to a PEM file,
 * but conceptually this is the ONE key that is trusted unconditionally
 * -- nothing verifies it; everything else is verified starting from it.
 */
#ifndef ROOT_OF_TRUST_H
#define ROOT_OF_TRUST_H

/* Returns the path to the ROM's trusted public key (the "OTP" key). */
const char *rot_public_key_path(void);

#endif /* ROOT_OF_TRUST_H */
