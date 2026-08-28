# Secure Boot Chain Simulator — `ROM → HSM firmware → BL2 → APP`

A small, readable, fully-working software simulation of an automotive-style
secure boot chain of trust. Built to run on **Ubuntu / WSL2 Ubuntu 24** with
nothing but `gcc`, `make`, `python3`, and OpenSSL. No hardware required.

This is **Stage 1** of a larger learning plan (secure boot lab). It focuses
only on the trust chain, signatures, key placement, rollback, and failure
handling — deliberately *not* on the full HSM service model or SecOC yet.

---

## The five things this simulates

1. **ROM verifies the HSM firmware** — the immutable Boot ROM checks the HSM
   firmware's signature against the root-of-trust key before anything else runs.
2. **HSM initializes only if its firmware is valid** — the HSM key store only
   unlocks *after* the HSM firmware has been verified.
3. **HSM verifies BL2** — the running HSM firmware verifies the second-stage
   bootloader using a key held *inside* the HSM (never handed back to the caller).
4. **BL2 verifies APP** — the second-stage bootloader verifies the application.
5. **Rollback + failure testing** — every stage also checks a monotonic version
   counter (anti-rollback), and the lab includes scripted failure scenarios
   (corrupted image, wrong signing key, version downgrade).

---

## The trust model (read this first)

A chain of trust has to start somewhere that is trusted *unconditionally* —
otherwise you'd need something to verify the verifier, forever. That starting
point is the **hardware root of trust**: immutable Boot ROM code plus a key
fused into OTP at manufacture. Nothing verifies the ROM; everything else is
verified starting *from* it.

```
ROM (root of trust, trusted unconditionally)
  │  verifies signature of ↓  using rom_root_pub.pem  (the "OTP" key)
HSM firmware
  │  runs only because ROM verified it
  │  unlocks its key store only now
  │  verifies signature of ↓  using a key held INSIDE the HSM
BL2 (second-stage bootloader)
  │  runs only because the HSM verified it
  │  verifies signature of ↓  using oem_app_pub.pem
APP (the application)
  │  runs only because BL2 verified it
```

### The two-chain idea

On real silicon the HSM is a *separate core* with its **own** boot ROM and its
**own** OTP key that verifies its **own** firmware — before it will touch any
operational keys. This lab models that ordering: the HSM key store
(`hsm_keystore.c`) refuses to do anything until `hsm_keystore_init()` is called,
and that only happens inside `hsm_fw_start()`, which ROM only calls after
verifying the HSM firmware's signature. So "keys unlock only after the HSM
firmware is verified" is enforced by construction.

---

## What is real vs simulated (honesty section)

| Concept | Real hardware | This lab |
|---|---|---|
| Boot ROM immutability | Burned into silicon | An ordinary C function that runs first |
| OTP root key | Fused into the chip | `keys/rom_root_pub.pem` on disk |
| Stage hand-off | Jump to a reset vector | One C function calling the next |
| HSM isolation | Hardware firewall between cores | An API that returns only pass/fail, never key bytes |
| Anti-rollback counter | Tamper-resistant HW counter / protected NVM | `state/rollback_counters.txt` |
| Crypto | HW crypto accelerator | OpenSSL software crypto (ECDSA-P256 + SHA-256) |

The **logic** of the trust chain is 100% real. The parts that need physical
silicon (immutability, memory isolation, tamper resistance) are modeled at the
level appropriate for learning the concepts — they can't be truly reproduced in
software, and that's the point, not a shortcut.

---

## Folder structure

```
secure_boot_lab/
├─ rom/           boot_rom.c/.h, root_of_trust.c/.h   — the root of trust
├─ hsm/           hsm_fw, hsm_verify, hsm_keystore, hsm_rollback  — the HSM core
├─ bl2/           bl2.c/.h, bl2_verify.c/.h            — second-stage bootloader
├─ app/           app_main.c/.h                        — the application
├─ common/        verify_common.c/.h                   — shared hash+sig+manifest logic
├─ images/        *.bin, *.sig, *_manifest.json        — firmware images + signatures
├─ keys/          *.pem  (+ signing_test_keys/)        — the key hierarchy
├─ tools/         generate_keys / sign_image / make_manifest / corrupt_image (Python)
├─ tests/         test_hsm_fw_verify / test_bl2_verify / test_app_verify / test_rollback
├─ state/         rollback_counters.txt                — persisted anti-rollback state
├─ bin/           build output
├─ main.c         entry point (power-on → Boot ROM)
├─ Makefile
├─ setup.sh       generate keys/images/signatures/manifests + negative artifacts
├─ run.sh         run happy path + 3 failure scenarios
└─ run_tests.sh   build + run all unit tests
```

### Additions beyond the original sketch (and why)

- **`common/`** — the hash + manifest + signature-verify logic is identical for
  ROM, HSM, and BL2. Factored out once instead of copied three times, exactly
  like real firmware does with a shared crypto library.
- **`state/`** — persists anti-rollback counters between runs (simulating the
  monotonic hardware counter).
- **`keys/oem_bl_priv.pem`, `keys/oem_app_priv.pem`** — the *private* halves
  needed to actually produce BL2/APP signatures (the original sketch listed only
  the public halves).
- **`main.c`, `run.sh`, `run_tests.sh`, `setup.sh`** — glue and drivers.

---

## The key hierarchy

| Signs | Private key | Verified with (public) | Held by |
|---|---|---|---|
| HSM firmware | `keys/hsm_signer_priv.pem` | `keys/rom_root_pub.pem` | ROM (the "OTP" root key) |
| BL2 | `keys/oem_bl_priv.pem` | `keys/oem_bl_pub.pem` | inside the HSM key store |
| APP | `keys/oem_app_priv.pem` | `keys/oem_app_pub.pem` | BL2 |
| (attacker) | `keys/signing_test_keys/attacker_priv.pem` | `attacker_pub.pem` | negative tests only |

All keys are ECDSA P-256. In reality the private keys live offline in a secure
signing facility and never touch the device; here they're on disk so you can
experiment.

---

## Quick start (Ubuntu / WSL2 Ubuntu 24)

```bash
# 1. one-time system deps
sudo apt update
sudo apt install -y build-essential python3 python3-pip libssl-dev
pip3 install --user cryptography      # or: pip3 install --break-system-packages cryptography

# 2. generate keys, images, signatures, manifests, and negative artifacts
./setup.sh

# 3. build the simulator
make

# 4. run the happy path
./bin/secure_boot_sim

# 5. run all four scenarios (happy path + 3 failures)
./run.sh

# 6. build and run the unit tests
make tests
./run_tests.sh
```

---

## The four scenarios (`./run.sh`)

1. **Happy path** — every image valid → boots all the way to APP.
2. **Corrupted BL2** — a bit-flipped BL2 → the HSM detects the hash mismatch and
   halts. ROM/HSM-fw stages before it still pass, showing exactly where the
   chain breaks.
3. **Wrong-key HSM firmware** — the HSM firmware re-signed with the attacker key
   → ROM's root key won't verify it, so the HSM never even starts. This is the
   most important scenario: it shows the keys stay locked when the firmware is
   untrusted.
4. **Rollback attack** — APP is updated to v2 (accepted), then an attacker tries
   to boot the older, still-validly-signed v1 → anti-rollback rejects it purely
   on version, even though its signature is perfectly valid.

To reset all rollback counters back to baseline, just re-run `./setup.sh`.

---

## Experiments to try (this is a lab — poke at it)

- Change a byte in `images/app.bin` by hand, re-run `./bin/secure_boot_sim`, and
  watch APP fail hash verification without touching the signature.
- Sign BL2 with the *app* key (`tools/sign_image.py`) and watch the HSM reject it
  — right image, wrong signing key.
- Bump APP to version 5 in its manifest but forget to re-sign — watch it fail.
- Add a second key to the HSM key store (`hsm/hsm_keystore.c`) and route BL2
  verification through it.
- Make the ROM also enforce a *minimum* version, not just monotonic.

---

## Where this goes next (your roadmap)

- **Stage 2 — HSM service model:** split the HSM into a real separate *process*
  with a client/server API over a socket or pipe, so "keys never leave the HSM"
  becomes a real process boundary, not just an API convention. This is where
  wolfHSM becomes worth adopting.
- **Stage 3 — automotive features:** SecOC (CMAC + freshness over `vcan0`), UDS
  security access + session/lifecycle state machine, secure update.

Stage 1 (this lab) deliberately uses a clean custom simulator instead of
wolfHSM, because writing each trust boundary yourself is the fastest way to
actually understand it.

---

## Notes

- Everything here runs as a normal user process. It is a **learning tool**, not
  a secure system — the "protected" key store and rollback counters are plain
  files. The value is in the trust-chain *logic*, which is faithful to how real
  secure boot works.
- Crypto is ECDSA P-256 over SHA-256 via OpenSSL's `EVP_DigestVerify`. The
  Python signer signs the raw image bytes (the library hashes once internally)
  so it matches the C verifier exactly.
