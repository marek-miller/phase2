/* xoshiro256ss.h: A thread-safe implementation of xoshiro256starstar PRNG. */
#ifndef XOSHIRO256SS_H
#define XOSHIRO256SS_H

/* -------------------------------------------------------------------------- *
 * A thin wrapper around the original implementation of Xoshiro256**          *
 * by Blackman and Vigna:                                                     *
 *                                                                            *
 *   David Blackman and Sebastiano Vigna. 2021.                               *
 *   Scrambled Linear Pseudorandom Number Generators.                         *
 *   ACM Trans. Math. Softw. 47, 4,                                           *
 *   Article 36 (December 2021), 32 pages.                                    *
 *   https://doi.org/10.1145/3460772                                          *
 *                                                                            *
 * The original implementation uses a static internal state for the PRNG.     *
 * The functions here take a caller-owned state instead, which makes them     *
 * thread-safe.                                                               *
 *                                                                            *
 * The implementation by Blackman and Vigna is part of Public Domain.         *
 * See <http://creativecommons.org/publicdomain/zero/1.0/>.                   *
 * -------------------------------------------------------------------------- */
#include <stdint.h>

struct xoshiro256ss {
	uint64_t s[4];
};

void xoshiro256ss_init(struct xoshiro256ss *rng, uint64_t seed);

uint64_t xoshiro256ss_next(struct xoshiro256ss *rng);

void xoshiro256ss_jump(struct xoshiro256ss *rng);

void xoshiro256ss_longjump(struct xoshiro256ss *rng);

/*
 * Obtain a double, uniformely distributed on the unit interval [0, 1].
 *
 * Taken from: https://prng.di.unimi.it/
 */
#define xoshiro256ss_dbl01(rng)                                                \
	((double)(xoshiro256ss_next(rng) >> 11) * 0x1.0p-53)

#endif /* XOSHIRO256SS_H */
