/****************************************************************
 *
 * The author of this software is David M. Gay.
 *
 * Copyright (c) 1991, 2000, 2001 by Lucent Technologies.
 * Copyright (c) 2017-2023 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2023 GAMS Development Corp. <support@gams.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose without fee is hereby granted, provided that this entire notice
 * is included in all copies of any software which is or includes a copy
 * or modification of this software and in all copies of the supporting
 * documentation for such software.
 *
 * THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTY.  IN PARTICULAR, NEITHER THE AUTHOR NOR LUCENT MAKES ANY
 * REPRESENTATION OR WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY
 * OF THIS SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR PURPOSE.
 *
 ***************************************************************/

/* On a machine with IEEE extended-precision registers, it is
 * necessary to specify double-precision (53-bit) rounding precision
 * before invoking strtod or dtoa.  If the machine uses (the equivalent
 * of) Intel 80x87 arithmetic, the call
 *      _control87(PC_53, MCW_PC);
 * does this with many compilers.  Whether this or another call is
 * appropriate depends on the compiler; for this to work, it may be
 * necessary to #include "float.h" or another system-dependent header
 * file.
 */

/* strtod for IEEE-arithmetic machines.
 * (Note that IEEE arithmetic is disabled by gcc's -ffast-math flag.)
 *
 * This strtod returns a nearest machine number to the input decimal
 * string (or sets errno to ERANGE).  With IEEE arithmetic, ties are
 * broken by the IEEE round-even rule.  Otherwise ties are broken by
 * biased rounding (add half and chop).
 *
 * Inspired loosely by William D. Clinger's paper "How to Read Floating
 * Point Numbers Accurately" [Proc. ACM SIGPLAN '90, pp. 92-101].
 *
 * Modifications:
 *
 *      1. We only require IEEE double-precision
 *              arithmetic (not IEEE double-extended).
 *      2. We get by with floating-point arithmetic in a case that
 *              Clinger missed -- when we're computing d * 10^n
 *              for a small integer d and the integer n is not too
 *              much larger than 22 (the maximum integer k for which
 *              we can represent 10^k exactly), we may be able to
 *              compute (d*10^k) * 10^(e-k) with just one roundoff.
 *      3. Rather than a bit-at-a-time adjustment of the binary
 *              result in the hard case, we use floating-point
 *              arithmetic to determine the adjustment to within
 *              one bit; only in really hard cases do we need to
 *              compute a second residual.
 *      4. Because of 3., we don't need a large table of powers of 10
 *              for ten-to-e (just some small tables, e.g. of 10^k
 *              for 0 <= k <= 22).
 */

/*
 * #define IEEE_8087 for IEEE-arithmetic machines where the least
 *      significant byte has the lowest address.
 * #define IEEE_MC68k for IEEE-arithmetic machines where the most
 *      significant byte has the lowest address.
 * #define No_leftright to omit left-right logic in fast floating-point
 *      computation of dtoa.  This will cause dtoa modes 4 and 5 to be
 *      treated the same as modes 2 and 3 for some inputs.
 * #define Honor_FLT_ROUNDS if FLT_ROUNDS can assume the values 2 or 3
 *      and strtod and dtoa should round accordingly.  Unless Trust_FLT_ROUNDS
 *      is also #defined, fegetround() will be queried for the rounding mode.
 *      Note that both FLT_ROUNDS and fegetround() are specified by the C99
 *      standard (and are specified to be consistent, with fesetround()
 *      affecting the value of FLT_ROUNDS), but that some (Linux) systems
 *      do not work correctly in this regard, so using fegetround() is more
 *      portable than using FLT_ROUNDS directly.
 * #define Check_FLT_ROUNDS if FLT_ROUNDS can assume the values 2 or 3
 *      and Honor_FLT_ROUNDS is not #defined.
 * #define ROUND_BIASED for IEEE-format with biased rounding and arithmetic
 *      that rounds toward +Infinity.
 * #define ROUND_BIASED_without_Round_Up for IEEE-format with biased
 *      rounding when the underlying floating-point arithmetic uses
 *      unbiased rounding.  This prevent using ordinary floating-point
 *      arithmetic when the result could be computed with one rounding error.
 * #define MALLOC your_malloc, where your_malloc(n) acts like malloc(n)
 *      if memory is available and otherwise does something you deem
 *      appropriate.  If MALLOC is undefined, malloc will be invoked
 *      directly -- and assumed always to succeed.
 * #define NO_INFNAN_CHECK if you do not wish to have INFNAN_CHECK
 *      #defined automatically on IEEE systems.  On such systems,
 *      when INFNAN_CHECK is #defined, strtod checks
 *      for Infinity and NaN (case insensitively).  On some systems
 *      (e.g., some HP systems), it may be necessary to #define NAN_WORD0
 *      appropriately -- to the most significant word of a quiet NaN.
 *      (On HP Series 700/800 machines, -DNAN_WORD0=0x7ff40000 works.)
 *      When INFNAN_CHECK is #defined and No_Hex_NaN is not #defined,
 *      strtod also accepts (case insensitively) strings of the form
 *      NaN(x), where x is a string of hexadecimal digits and spaces;
 *      if there is only one string of hexadecimal digits, it is taken
 *      for the 52 fraction bits of the resulting NaN; if there are two
 *      or more strings of hex digits, the first is for the high 20 bits,
 *      the second and subsequent for the low 32 bits, with intervening
 *      white space ignored; but if this results in none of the 52
 *      fraction bits being on (an IEEE Infinity symbol), then NAN_WORD0
 *      and NAN_WORD1 are used instead.
 * #define USE_LOCALE to use the current locale's decimal_point value.
 * #define SET_INEXACT if IEEE arithmetic is being used and extra
 *      computation should be done to set the inexact flag when the
 *      result is inexact and avoid setting inexact when the result
 *      is exact.  In this case, dtoa.c must be compiled in
 *      an environment, perhaps provided by #include "dtoa.c" in a
 *      suitable wrapper, that defines two functions,
 *              int get_inexact(void);
 *              void clear_inexact(void);
 *      such that get_inexact() returns a nonzero value if the
 *      inexact bit is already set, and clear_inexact() sets the
 *      inexact bit to 0.  When SET_INEXACT is #defined, strtod
 *      also does extra computations to set the underflow and overflow
 *      flags when appropriate (i.e., when the result is tiny and
 *      inexact or when it is a numeric value rounded to +-infinity).
 * #define NO_HEX_FP to omit recognition of hexadecimal floating-point
 *      values by strtod.
 * #define NO_STRTOD_BIGCOMP (on IEEE-arithmetic systems only for now)
 *      to disable logic for "fast" testing of very long input strings
 *      to strtod.  This testing proceeds by initially truncating the
 *      input string, then if necessary comparing the whole string with
 *      a decimal expansion to decide close cases. This logic is only
 *      used for input more than STRTOD_DIGLIM digits long (default 40).
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#ifdef DEBUG
#define Bug(x) {fprintf(stderr, "%s\n", x); exit(1);}
#endif

/* sometimes we put in a quick hack, sometimes we really mean to assert */
#define HACKASSERT assert
#define GOODASSERT assert

/* should never get compiled in */
#define MALLOC noMalloc

#ifdef USE_LOCALE
#include <locale.h>
#endif

#ifdef Honor_FLT_ROUNDS
#ifndef Trust_FLT_ROUNDS
#include <fenv.h>
#endif
#endif

#define NO_STRTOD_BIGCOMP       /* IMHO the bigcomp stuff is buggy */

/* we will set these automagically */
#undef IEEE_8087
#undef IEEE_MC68k
#undef Sudden_Underflow

#if defined(_WIN32)
# define IEEE_8087
# if defined(__INTEL_COMPILER_BUILD_DATE)
#   define Sudden_Underflow
# endif
#elif defined(__linux) || defined(__linux__) || defined(__APPLE__)
# define IEEE_8087
#elif defined(__sparc) || (__HOS_AIX__)
# define IEEE_MC68k
#elif defined(__sun)
# define IEEE_8087
#else
# error This environment not recognized as little- or big-endian
#endif

#if defined(IEEE_8087) + defined(IEEE_MC68k) != 1
# error Exactly one of IEEE_8087 or IEEE_MC68k should be defined.
#endif

#ifdef IEEE_MC68k
#endif
#ifdef IEEE_8087
#endif

#ifndef NO_INFNAN_CHECK
# undef INFNAN_CHECK
# define INFNAN_CHECK
#endif

#include "errno.h"
#include "float.h"

#include "math.h"

#if defined(DTOA_EXPORTS)
# include "dtoaLib.h"
#else
# include "dtoaLoc.h"
#endif

typedef union { double d; uint32_t L[2]; } U;

#ifdef IEEE_8087
# define word0(x) (x)->L[1]
# define word1(x) (x)->L[0]
#else
# define word0(x) (x)->L[0]
# define word1(x) (x)->L[1]
#endif
#define dval(x) (x)->d

#ifndef STRTOD_DIGLIM
#define STRTOD_DIGLIM 40
#endif

#ifdef DIGLIM_DEBUG
extern int strtod_diglim;
#else
#define strtod_diglim STRTOD_DIGLIM
#endif

/* DTOOA_USE_ND_BOUND will always get defined: treat it as a boolean value
 * that enables a bound on the number of digits treated as significant.
 * This is handy to get a bound on the amount of space required to convert
 * long strings to doubles.
 * Excess digits are treated as if they were zeros.
 * Why 72?  The heap makes a jump going to 73, but we save little
 * space by making the limit less than 72.
 */

#if defined(DTOA_ND_BOUND)      /* implies we will use it */
# if 0 == DTOA_USE_ND_BOUND
#  error "Confusing: DTOA_USE_ND_BOUND is 0 but DTOA_ND_BOUND is specified"
# endif
# undef  DTOA_USE_ND_BOUND
# define DTOA_USE_ND_BOUND 1
#else
# if (! defined(DTOA_USE_ND_BOUND)) || (0 != DTOA_USE_ND_BOUND)
#  undef  DTOA_USE_ND_BOUND
#  define DTOA_USE_ND_BOUND 1
#  define DTOA_ND_BOUND  72
# endif
#endif

/* #define P DBL_MANT_DIG */
/* Ten_pmax = floor(P*log(2)/log(5)) */
/* Bletch = (highest power of 2 < DBL_MAX_10_EXP) / 16 */
/* Quick_max = floor((P-1)*log(FLT_RADIX)/log(10) - 1) */
/* Int_max = floor(P*log(FLT_RADIX)/log(10) - 1) */

#define Exp_shift  20
#define Exp_shift1 20
#define Exp_msk1    0x100000
#define Exp_msk11   0x100000
#define Exp_mask  0x7ff00000
#define P 53
#define Nbits 53
#define Bias 1023
#define Emax 1023
#define Emin (-1022)
#define Exp_1  0x3ff00000
#define Exp_11 0x3ff00000
#define Ebits 11
#define Frac_mask  0xfffff
#define Frac_mask1 0xfffff
#define Ten_pmax 22
#define Bletch 0x10
#define Bndry_mask  0xfffff
#define Bndry_mask1 0xfffff
#define LSB 1
#define Sign_bit 0x80000000
#define Log2P 1
#define Tiny0 0
#define Tiny1 1
#define Quick_max 14
#define Int_max 14
#ifdef Flush_Denorm     /* debugging option */
# undef Sudden_Underflow
#endif

#ifndef Flt_Rounds
# ifdef FLT_ROUNDS
#  define Flt_Rounds FLT_ROUNDS
# else
#  define Flt_Rounds 1
# endif
#endif /*Flt_Rounds*/

#ifdef Honor_FLT_ROUNDS
# undef Check_FLT_ROUNDS
# define Check_FLT_ROUNDS
#else
# define Rounding Flt_Rounds
#endif

#ifdef ROUND_BIASED_without_Round_Up
# undef  ROUND_BIASED
# define ROUND_BIASED
#endif

#define rounded_product(a,b) a *= b
#define rounded_quotient(a,b) a /= b

#define Big0 (Frac_mask1 | Exp_msk1*(DBL_MAX_EXP+Bias-1))
#define Big1 0xffffffff

typedef struct BCinfo {
  int dp0, dp1, dplen, dsign;
  int e0, inexact, nd, nd0, rounding, scale, uflchk;
} BCinfo_t;

#define FFFFFFFF 0xffffffffUL


#define Kmax 7

struct Bigint {
  struct Bigint *next;
  int k, maxwds, sign, wds;
  uint32_t x[1];
};
typedef struct Bigint Bigint;

#if ! defined(HEAP_SZ)
# define HEAP_SZ 200
/* roughly, HEAP_SZ * 8 = PRIVATE_MEM, but we can use less now that
 * pow5mult is using static memory: the original value of
 * PRIVATE_MEM = 288 * 8 is excessively conservative
 * FYI, PRIVATE_MEM is the memory used in the original.
 */
#endif
typedef struct bigHeap {
  double base[HEAP_SZ];
  double *next;
  Bigint *freeList[Kmax+1];
} bigHeap_t;

#if defined(DTOA_INFO_LOC)
static unsigned int heapHW;     /* highwater mark for heap */
static int kHW;                 /* highwater mark for k */
#endif

static void
heapInit (bigHeap_t *hp)
{
  // (void) memset(hp, 0, sizeof(bigHeap_t));
  (void) memset(hp->freeList, 0, sizeof(Bigint *) * (Kmax+1));
  hp->next = hp->base;
} /* heapInit */

static Bigint *
Balloc (bigHeap_t *hp, int k)
{
  int x;
  Bigint *rv;
  unsigned int len;

  GOODASSERT(k <= Kmax);

  if ((rv = hp->freeList[k]))
    hp->freeList[k] = rv->next;
  else {
    x = 1 << k;
    len = (sizeof(Bigint) + (x-1)*sizeof(uint32_t) + sizeof(double) - 1)
      / sizeof(double);
    GOODASSERT(HEAP_SZ - (hp->next - hp->base) >= len); /* should never overflow the heap */
    rv = (Bigint *) hp->next;
    hp->next += len;
#if defined(DTOA_INFO_LOC)
    if (k > kHW)
      kHW = k;
    if ((len = (hp->next - hp->base)) > heapHW)
      heapHW = len;
#endif
    rv->k = k;
    rv->maxwds = x;
  }
  rv->sign = rv->wds = 0;
  return rv;
} /* Balloc */

static void
Bfree (bigHeap_t *hp, Bigint *v)
{
  if (v) {
    GOODASSERT(v->k <= Kmax);
    v->next = hp->freeList[v->k];
    hp->freeList[v->k] = v;
  }
} /* Bfree */

#define BCOPY(x,y) memcpy((void *)&x->sign, (void *)&y->sign, \
y->wds*sizeof(int32_t) + 2*sizeof(int))

static Bigint *
multadd (bigHeap_t *hp, Bigint *b, int m, int a) /* multiply by m and add a */
{
  int i, wds;
  uint32_t *x;
  uint64_t carry, y;
  Bigint *b1;

  wds = b->wds;
  x = b->x;
  i = 0;
  carry = a;
  do {
    y = *x * (uint64_t)m + carry;
    carry = y >> 32;
    *x++ = y & FFFFFFFF;
  } while(++i < wds);
  if (carry) {
    if (wds >= b->maxwds) {
      b1 = Balloc(hp, b->k+1);
      BCOPY(b1, b);
      Bfree(hp, b);
      b = b1;
    }
    b->x[wds++] = (uint32_t)carry;
    b->wds = wds;
  }
  return b;
} /* multadd */

static Bigint *
s2b (bigHeap_t *hp, const char *s, int nd0, int nd, uint32_t y9, int dplen)
{
  Bigint *b;
  int i, k;
  int32_t x, y;

  x = (nd + 8) / 9;
  for (k = 0, y = 1;  x > y;  y <<= 1, k++);
  b = Balloc(hp, k);
  b->x[0] = y9;
  b->wds = 1;

  i = 9;
  if (9 < nd0) {
    s += 9;
    do
      b = multadd(hp, b, 10, *s++ - '0');
    while(++i < nd0);
    s += dplen;
  }
  else
    s += dplen + 9;
  for ( ;  i < nd;  i++)
    b = multadd(hp, b, 10, *s++ - '0');
  return b;
} /* s2b */

static int
hi0bits (uint32_t x)
{
  int k = 0;

  if (!(x & 0xffff0000)) {
    k = 16;
    x <<= 16;
  }
  if (!(x & 0xff000000)) {
    k += 8;
    x <<= 8;
  }
  if (!(x & 0xf0000000)) {
    k += 4;
    x <<= 4;
  }
  if (!(x & 0xc0000000)) {
    k += 2;
    x <<= 2;
  }
  if (!(x & 0x80000000)) {
    k++;
    if (!(x & 0x40000000))
      return 32;
  }
  return k;
} /* hi0bits */

static int
lo0bits (uint32_t *y)
{
  int k;
  uint32_t x = *y;

  if (x & 7) {
    if (x & 1)
      return 0;
    if (x & 2) {
      *y = x >> 1;
      return 1;
    }
    *y = x >> 2;
    return 2;
  }
  k = 0;
  if (!(x & 0xffff)) {
    k = 16;
    x >>= 16;
  }
  if (!(x & 0xff)) {
    k += 8;
    x >>= 8;
  }
  if (!(x & 0xf)) {
    k += 4;
    x >>= 4;
  }
  if (!(x & 0x3)) {
    k += 2;
    x >>= 2;
  }
  if (!(x & 1)) {
    k++;
    x >>= 1;
    if (!x)
      return 32;
  }
  *y = x;
  return k;
} /* lo0bits */

static Bigint *
i2b (bigHeap_t *hp, int i)
{
  Bigint *b;

  b = Balloc(hp, 1);
  b->x[0] = i;
  b->wds = 1;
  return b;
} /* i2b */

static Bigint *
mult (bigHeap_t *hp, const Bigint *a, const Bigint *b)
{
  Bigint *c;
  int k, wa, wb, wc;
  const uint32_t *x, *xa, *xae, *xb, *xbe;
  uint32_t *xc, *xc0;
  uint32_t y;
  uint64_t carry, z;

  if (a->wds < b->wds) {
    const Bigint *swp = a;
    a = b;
    b = swp;
  }
  k = a->k;
  wa = a->wds;
  wb = b->wds;
  wc = wa + wb;
  if (wc > a->maxwds)
    k++;
  c = Balloc(hp, k);
  for (xc = c->x, xc0 = xc + wc;  xc < xc0;  xc++)
    *xc = 0;
  xa = a->x;
  xae = xa + wa;
  xb = b->x;
  xbe = xb + wb;
  xc0 = c->x;
  for ( ;  xb < xbe;  xc0++) {
    if ((y = *xb++)) {
      x = xa;
      xc = xc0;
      carry = 0;
      do {
        z = *x++ * (uint64_t)y + *xc + carry;
        carry = z >> 32;
        *xc++ = z & FFFFFFFF;
      } while(x < xae);
      *xc = (uint32_t)carry;
    }
  }
  for (xc0 = c->x, xc = xc0 + wc;  wc > 0 && !*--xc;  --wc);
  c->wds = wc;
  return c;
} /* mult */

struct Bigint5 {
  struct Bigint *next;
  int k, maxwds, sign, wds;
  uint32_t x[5];
};
typedef struct Bigint5 Bigint5_t;

struct Bigint20 {
  struct Bigint *next;
  int k, maxwds, sign, wds;
  uint32_t x[20];
};
typedef struct Bigint20 Bigint20_t;

static Bigint *
pow5mult (bigHeap_t *hp, Bigint *b, int k)
{
  Bigint *bb;
  const Bigint *pw, *prev;
  int i;
  static const int p05[] = { 5, 25, 125, 625, 3125, 15625, 78125 };
  static const Bigint20_t p256 = {NULL,      /* next */
                     5,         /* k */
                     32,        /* maxwds */
                     0,         /* sign */
                     19,        /* wds */
                     {0x982e7c01, 0xbed3875b, 0xd8d99f72, 0x12152f87, 0x6bde50c6,
                      0xcf4a6e70, 0xd595d80f, 0x26b2716e, 0xadc666b0, 0x1d153624,
                      0x3c42d35a, 0x63ff540e, 0xcc5573c0, 0x65f9ef17, 0x55bc28f2,
                      0x80dcc7f7, 0xf46eeddc, 0x5fdcefce, 0x000553f7}
  };                            /* 5^256 */
  static const Bigint20_t p128 = {(Bigint *)&p256, /* next */
                     4,         /* k */
                     16,        /* maxwds */
                     0,         /* sign */
                     10,        /* wds */
                     {0x2e953e01, 0x03df9909, 0x0f1538fd, 0x2374e42f, 0xd3cff5ec,
                      0xc404dc08, 0xbccdb0da, 0xa6337f19, 0xe91f2603, 0x0000024e}
  };                            /* 5^128 */
  static const Bigint5_t  p64 = {(Bigint *)&p128, /* next */
                    3,          /* k */
                    8,          /* maxwds */
                    0,          /* sign */
                    5,          /* wds */
                    {0xbf6a1f01, 0x6e38ed64, 0xdaa797ed, 0xe93ff9f4, 0x00184f03}
  };                            /* 5^64 */
  static const Bigint5_t  p32 = {(Bigint *)&p64, /* next */
                    2,          /* k */
                    4,          /* maxwds */
                    0,          /* sign */
                    3,          /* wds */
                    {0x85acef81, 0x2d6d415b, 0x000004ee}
  };                            /* 5^32 */
  static const Bigint5_t  p16 = {(Bigint *)&p32, /* next */
                    1,          /* k */
                    2,          /* maxwds */
                    0,          /* sign */
                    2,          /* wds */
                    {0x86f26fc1, 0x00000023}
  };                            /* 5^16 */

  if ((i = k & 7))
    b = multadd(hp, b, p05[i-1], 0);
  if (k & 8)
    b = multadd(hp, b, 390625, 0);
  if (!(k >>= 4))
    return b;

  /* if we get here, we want b *= (390625^2)^k */
#if 0
  /* do it quick and dirty */
  for (i = 0;  i < k;  i++) {
    b = multadd(hp, b, 390625, 0);
    b = multadd(hp, b, 390625, 0);
  }
#else
  for (pw = (Bigint * ) &p16;  pw;  pw = pw->next) {
    if (k & 1) {
      bb = mult(hp, b, pw);
      Bfree(hp, b);
      b = bb;
    }
    if (!(k >>= 1))
      return b;
    prev = pw;
  }
  /* if we get here, the table wasn't big enough */
  k <<= 1;
  for (i = 0;  i < k;  i++) {
    bb = mult(hp, b, prev);
    Bfree(hp, b);
    b = bb;
  }
#endif
  return b;
} /* pow5mult */

static Bigint *
lshift (bigHeap_t *hp, Bigint *b, int k)
{
  int i, k1, n, n1;
  Bigint *b1;
  uint32_t *x, *x1, *xe, z;

  n = k >> 5;
  k1 = b->k;
  n1 = n + b->wds + 1;
  for (i = b->maxwds;  n1 > i;  i <<= 1)
    k1++;
  b1 = Balloc(hp, k1);
  x1 = b1->x;
  for (i = 0;  i < n;  i++)
    *x1++ = 0;
  x = b->x;
  xe = x + b->wds;
  if (k &= 0x1f) {
    k1 = 32 - k;
    z = 0;
    do {
      *x1++ = *x << k | z;
      z = *x++ >> k1;
    } while (x < xe);
    if ((*x1 = z))
      ++n1;
  }
  else
    do
      *x1++ = *x++;
    while (x < xe);
  b1->wds = n1 - 1;
  Bfree(hp, b);
  return b1;
} /* lshift */

static int
cmp (const Bigint *a, const Bigint *b)
{
  const uint32_t *xa, *xa0, *xb, *xb0;
  int i, j;

  i = a->wds;
  j = b->wds;
#ifdef DEBUG
  if (i > 1 && !a->x[i-1])
    Bug("cmp called with a->x[a->wds-1] == 0");
  if (j > 1 && !b->x[j-1])
    Bug("cmp called with b->x[b->wds-1] == 0");
#endif
  if (i -= j)
    return i;
  xa0 = a->x;
  xa = xa0 + j;
  xb0 = b->x;
  xb = xb0 + j;
  for ( ; ; ) {
    if (*--xa != *--xb)
      return *xa < *xb ? -1 : 1;
    if (xa <= xa0)
      break;
  }
  return 0;
} /* cmp */

static Bigint *
diff (bigHeap_t *hp, const Bigint *a, const Bigint *b)
{
  Bigint *c;
  int i, wa, wb;
  const uint32_t *xa, *xae, *xb, *xbe;
  uint32_t *xc;
  uint64_t borrow, y;

  i = cmp(a,b);
  if (!i) {
    c = Balloc(hp, 0);
    c->wds = 1;
    c->x[0] = 0;
    return c;
  }
  if (i < 0) {
    const Bigint *swp = a;
    a = b;
    b = swp;
    i = 1;
  }
  else
    i = 0;
  c = Balloc(hp, a->k);
  c->sign = i;
  wa = a->wds;
  xa = a->x;
  xae = xa + wa;
  wb = b->wds;
  xb = b->x;
  xbe = xb + wb;
  xc = c->x;
  borrow = 0;
  do {
    y = (uint64_t)*xa++ - *xb++ - borrow;
    borrow = y >> 32 & (uint32_t)1;
    *xc++ = y & FFFFFFFF;
  }
  while(xb < xbe);
  while(xa < xae) {
    y = *xa++ - borrow;
    borrow = y >> 32 & (uint32_t)1;
    *xc++ = y & FFFFFFFF;
  }
  while(!*--xc)
    wa--;
  c->wds = wa;
  return c;
} /* diff */

static double
ulp (U *x)
{
  int32_t L;
  U u;

  L = (word0(x) & Exp_mask) - (P-1)*Exp_msk1;
  word0(&u) = L;
  word1(&u) = 0;
  return dval(&u);
} /* ulp */

static double
b2d (Bigint *a, int *e)
{
  uint32_t *xa, *xa0, w, y, z;
  int k;
  U d;
#define d0 word0(&d)
#define d1 word1(&d)

  xa0 = a->x;
  xa = xa0 + a->wds;
  y = *--xa;
#ifdef DEBUG
  if (!y) Bug("zero y in b2d");
#endif
  k = hi0bits(y);
  *e = 32 - k;
  if (k < Ebits) {
    d0 = Exp_1 | y >> (Ebits - k);
    w = xa > xa0 ? *--xa : 0;
    d1 = y << ((32-Ebits) + k) | w >> (Ebits - k);
    goto ret_d;
  }
  z = xa > xa0 ? *--xa : 0;
  if (k -= Ebits) {
    d0 = Exp_1 | y << k | z >> (32 - k);
    y = xa > xa0 ? *--xa : 0;
    d1 = z << k | y >> (32 - k);
  }
  else {
    d0 = Exp_1 | y;
    d1 = z;
  }
 ret_d:
#undef d0
#undef d1
  return dval(&d);
} /* b2d */

static Bigint *
d2b (bigHeap_t *hp, U *d, int *e, int *bits)
{
  Bigint *b;
  int de, k;
  uint32_t *x, y, z;
#ifndef Sudden_Underflow
  int i;
#endif
#define d0 word0(d)
#define d1 word1(d)

  b = Balloc(hp, 1);
  x = b->x;

  z = d0 & Frac_mask;
  d0 &= 0x7fffffff;       /* clear sign bit, which we ignore */
#ifdef Sudden_Underflow
  de = (int)(d0 >> Exp_shift);
  z |= Exp_msk11;
#else
  if ((de = (int)(d0 >> Exp_shift)))
    z |= Exp_msk1;
#endif
  if ((y = d1)) {
    if ((k = lo0bits(&y))) {
      x[0] = y | z << (32 - k);
      z >>= k;
    }
    else
      x[0] = y;
#ifndef Sudden_Underflow
    i =
#endif
      b->wds = (x[1] = z) ? 2 : 1;
  }
  else {
    k = lo0bits(&z);
    x[0] = z;
#ifndef Sudden_Underflow
    i =
#endif
      b->wds = 1;
    k += 32;
  }
#ifndef Sudden_Underflow
  if (de) {
#endif
    *e = de - Bias - (P-1) + k;
    *bits = P - k;
#ifndef Sudden_Underflow
  }
  else {
    *e = de - Bias - (P-1) + 1 + k;
    *bits = 32*i - hi0bits(x[i-1]);
  }
#endif
  return b;
} /* d2b */
#undef d0
#undef d1

static double
ratio (Bigint *a, Bigint *b)
{
  U da, db;
  int k, ka, kb;

  dval(&da) = b2d(a, &ka);
  dval(&db) = b2d(b, &kb);
  k = ka - kb + 32*(a->wds - b->wds);
  if (k > 0)
    word0(&da) += k*Exp_msk1;
  else {
    k = -k;
    word0(&db) += k*Exp_msk1;
  }
  return dval(&da) / dval(&db);
} /* ratio */

static const double
tens[] = {1e0,  1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9,
          1e10, 1e11, 1e12, 1e13, 1e14, 1e15, 1e16, 1e17, 1e18, 1e19,
          1e20, 1e21, 1e22
          };

static const double bigtens[] = { 1e16, 1e32, 1e64, 1e128, 1e256 };
static const double tinytens[] = { 1e-16, 1e-32, 1e-64, 1e-128,
                9007199254740992.*9007199254740992.e-256
                /* = 2^106 * 1e-256 */
};
/* The factor of 2^53 in tinytens[4] helps us avoid setting the underflow */
/* flag unnecessarily.  It leads to a song and dance at the end of strtod. */
#define Scale_Bit 0x10
#define n_bigtens 5

#undef Need_Hexdig
#ifdef INFNAN_CHECK
#ifndef No_Hex_NaN
#define Need_Hexdig
#endif
#endif

#ifndef Need_Hexdig
#ifndef NO_HEX_FP
#define Need_Hexdig
#endif
#endif

#ifdef Need_Hexdig /*{*/
static unsigned char hexdig[256] = {
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        16,17,18,19,20,21,22,23,24,25,0,0,0,0,0,0,
        0,26,27,28,29,30,31,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,26,27,28,29,30,31,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
        };
#endif /* } Need_Hexdig */

#ifdef INFNAN_CHECK

#ifndef NAN_WORD0
#define NAN_WORD0 0x7ff80000
#endif

#ifndef NAN_WORD1
#define NAN_WORD1 0
#endif

static int
match (const char **sp, const char *t)
{
  int c, d;
  const char *s = *sp;

  while((d = *t++)) {
    if ((c = *++s) >= 'A' && c <= 'Z')
      c += 'a' - 'A';
    if (c != d)
      return 0;
  }
  *sp = s + 1;
  return 1;
} /* match */

#ifndef No_Hex_NaN
static void
hexnan (U *rvp, const char **sp)
{
  uint32_t c, x[2];
  const char *s;
  int c1, havedig, udx0, xshift;

  x[0] = x[1] = 0;
  havedig = xshift = 0;
  udx0 = 1;
  s = *sp;
  /* allow optional initial 0x or 0X */
  while((c = *(const unsigned char*)(s+1)) && c <= ' ')
    ++s;
  if (s[1] == '0' && (s[2] == 'x' || s[2] == 'X'))
    s += 2;
  while((c = *(const unsigned char*)++s)) {
    if ((c1 = hexdig[c]))
      c  = c1 & 0xf;
    else if (c <= ' ') {
      if (udx0 && havedig) {
        udx0 = 0;
        xshift = 1;
      }
      continue;
    }
#ifdef GDTOA_NON_PEDANTIC_NANCHECK
    else if (/*(*/ c == ')' && havedig) {
      *sp = s + 1;
      break;
    }
    else
      return; /* invalid form: don't change *sp */
#else
    else {
      do {
        if (/*(*/ c == ')') {
          *sp = s + 1;
          break;
        }
      } while((c = *++s));
      break;
    }
#endif
    havedig = 1;
    if (xshift) {
      xshift = 0;
      x[0] = x[1];
      x[1] = 0;
    }
    if (udx0)
      x[0] = (x[0] << 4) | (x[1] >> 28);
    x[1] = (x[1] << 4) | c;
  }
  if ((x[0] &= 0xfffff) || x[1]) {
    word0(rvp) = Exp_mask | x[0];
    word1(rvp) = x[1];
  }
}      /* hexnan */
#endif /*No_Hex_NaN*/
#endif /* INFNAN_CHECK */

#define ULbits 32
#define kshift 5
#define kmask 31

#if !defined(NO_HEX_FP) || defined(Honor_FLT_ROUNDS) /*{*/
static Bigint *
increment(bigHeap_t *hp, Bigint *b)
{
  uint32_t *x, *xe;
  Bigint *b1;

  x = b->x;
  xe = x + b->wds;
  do {
    if (*x < (uint32_t)0xffffffffL) {
      ++*x;
      return b;
    }
    *x++ = 0;
  } while(x < xe);
  if (b->wds >= b->maxwds) {
    b1 = Balloc(hp, b->k+1);
    BCOPY(b1,b);
    Bfree(hp, b);
    b = b1;
  }
  b->x[b->wds++] = 1;
  return b;
} /* increment */

#endif /*}*/

#ifndef NO_HEX_FP /*{*/

static void
rshift(Bigint *b, int k)
{
  uint32_t *x, *x1, *xe, y;
  int n;

  x = x1 = b->x;
  n = k >> kshift;
  if (n < b->wds) {
    xe = x + b->wds;
    x += n;
    if (k &= kmask) {
      n = 32 - k;
      y = *x++ >> k;
      while(x < xe) {
        *x1++ = (y | (*x << n)) & 0xffffffff;
        y = *x++ >> k;
      }
      if ((*x1 = y) !=0)
        x1++;
    }
    else
      while(x < xe)
        *x1++ = *x++;
  }
  if ((b->wds = (int) (x1 - b->x)) == 0)
    b->x[0] = 0;
} /* rshift */

static uint32_t
any_on(Bigint *b, int k)
{
  int n, nwds;
  uint32_t *x, *x0, x1, x2;

  x = b->x;
  nwds = b->wds;
  n = k >> kshift;
  if (n > nwds)
    n = nwds;
  else if (n < nwds && (k &= kmask)) {
    x1 = x2 = x[n];
    x1 >>= k;
    x1 <<= k;
    if (x1 != x2)
      return 1;
  }
  x0 = x;
  x += n;
  while(x > x0)
    if (*--x)
      return 1;
  return 0;
} /* any_on */

enum {  /* rounding values: same as FLT_ROUNDS */
  Round_zero = 0,
  Round_near = 1,
  Round_up = 2,
  Round_down = 3
};

static void
gethex(bigHeap_t *hp, const char **sp, U *rvp, int rounding, int sign)
{
  Bigint *b = NULL;
  const unsigned char *decpt, *s0, *s, *s1;
  int32_t e, e1;
  uint32_t L, lostbits, *x;
  int big, denorm, esign, havedig, k, n, nbits, up, zret;
  enum {
    emax = 0x7fe - Bias - P + 1,
    emin = Emin - P + 1
  };
#ifdef USE_LOCALE
  int i;
#ifdef NO_LOCALE_CACHE
  const unsigned char *decimalpoint = (unsigned char*)
    localeconv()->decimal_point;
#else
  const unsigned char *decimalpoint;
  static unsigned char *decimalpoint_cache;
  if (!(s0 = decimalpoint_cache)) {
    s0 = (unsigned char*)localeconv()->decimal_point;
    if ((decimalpoint_cache = (unsigned char*)
         MALLOC(strlen((const char*)s0) + 1))) {
      strcpy((char*)decimalpoint_cache, (const char*)s0);
      s0 = decimalpoint_cache;
    }
  }
  decimalpoint = s0;
#endif
#endif

  havedig = 0;
  s0 = *(const unsigned char **)sp + 2;
  while(s0[havedig] == '0')
    havedig++;
  s0 += havedig;
  s = s0;
  decpt = 0;
  zret = 0;
  e = 0;
  if (hexdig[*s])
    havedig++;
  else {
    zret = 1;
#ifdef USE_LOCALE
    for(i = 0; decimalpoint[i]; ++i) {
      if (s[i] != decimalpoint[i])
        goto pcheck;
    }
    decpt = s += i;
#else
    if (*s != '.')
      goto pcheck;
    decpt = ++s;
#endif
    if (!hexdig[*s])
      goto pcheck;
    while(*s == '0')
      s++;
    if (hexdig[*s])
      zret = 0;
    havedig = 1;
    s0 = s;
  }
  while(hexdig[*s])
    s++;
#ifdef USE_LOCALE
  if (*s == *decimalpoint && !decpt) {
    for (i = 1;  decimalpoint[i];  ++i) {
      if (s[i] != decimalpoint[i])
        goto pcheck;
    }
    decpt = s += i;
    while(hexdig[*s])
      s++;
  }/*}*/
#else
  if (*s == '.' && !decpt) {
    decpt = ++s;
    while(hexdig[*s])
      s++;
  }/*}*/
#endif
  if (decpt)
    e = -(((int32_t)(s-decpt)) << 2);
 pcheck:
  s1 = s;
  big = esign = 0;
  switch(*s) {
  case 'p':
  case 'P':
    switch(*++s) {
    case '-':
      esign = 1;
      /* no break */
    case '+':
      s++;
    }
    if ((n = hexdig[*s]) == 0 || n > 0x19) {
      s = s1;
      break;
    }
    e1 = n - 0x10;
    while((n = hexdig[*++s]) !=0 && n <= 0x19) {
      if (e1 & 0xf8000000)
        big = 1;
      e1 = 10*e1 + n - 0x10;
    }
    if (esign)
      e1 = -e1;
    e += e1;
  }
  *sp = (char*)s;
  if (!havedig)
    *sp = (char*)s0 - 1;
  if (zret)
    goto retz1;
  if (big) {
    if (esign) {
      switch(rounding) {
      case Round_up:
        if (sign)
          break;
        goto ret_tiny;
      case Round_down:
        if (!sign)
          break;
        goto ret_tiny;
      }
      goto retz;
    ret_tinyf:
      Bfree(hp, b);
    ret_tiny:
      errno = ERANGE;
      word0(rvp) = 0;
      word1(rvp) = 1;
      return;
    }
    switch(rounding) {
    case Round_near:
      goto ovfl1;
    case Round_up:
      if (!sign)
        goto ovfl1;
      goto ret_big;
    case Round_down:
      if (sign)
        goto ovfl1;
      goto ret_big;
    }
  ret_big:
    word0(rvp) = Big0;
    word1(rvp) = Big1;
    return;
  }
  n = (int) (s1 - s0 - 1);
  for(k = 0; n > (1 << (kshift-2)) - 1; n >>= 1)
    k++;
  b = Balloc(hp, k);
  x = b->x;
  n = 0;
  L = 0;
#ifdef USE_LOCALE
  for(i = 0; decimalpoint[i+1]; ++i);
#endif
  while(s1 > s0) {
#ifdef USE_LOCALE
    if (*--s1 == decimalpoint[i]) {
      s1 -= i;
      continue;
    }
#else
    if (*--s1 == '.')
      continue;
#endif
    if (n == ULbits) {
      *x++ = L;
      L = 0;
      n = 0;
    }
    L |= (hexdig[*s1] & 0x0f) << n;
    n += 4;
  }
  *x++ = L;
  b->wds = n = (int) (x - b->x);
  n = ULbits*n - hi0bits(L);
  nbits = Nbits;
  lostbits = 0;
  x = b->x;
  if (n > nbits) {
    n -= nbits;
    if (any_on(b,n)) {
      lostbits = 1;
      k = n - 1;
      if (x[k>>kshift] & 1 << (k & kmask)) {
        lostbits = 2;
        if (k > 0 && any_on(b,k))
          lostbits = 3;
      }
    }
    rshift(b, n);
    e += n;
  }
  else if (n < nbits) {
    n = nbits - n;
    b = lshift(hp, b, n);
    e -= n;
    x = b->x;
  }
  if (e > Emax) {
 ovfl:
    Bfree(hp, b);
 ovfl1:
    errno = ERANGE;
    word0(rvp) = Exp_mask;
    word1(rvp) = 0;
    return;
  }
  denorm = 0;
  if (e < emin) {
    denorm = 1;
    n = emin - e;
    if (n >= nbits) {
      switch (rounding) {
      case Round_near:
        if (n == nbits && (n < 2 || any_on(b,n-1)))
          goto ret_tinyf;
        break;
      case Round_up:
        if (!sign)
          goto ret_tinyf;
        break;
      case Round_down:
        if (sign)
          goto ret_tinyf;
      }
      Bfree(hp, b);
    retz:
      errno = ERANGE;
    retz1:
      rvp->d = 0.;
      return;
    }
    k = n - 1;
    if (lostbits)
      lostbits = 1;
    else if (k > 0)
      lostbits = any_on(b,k);
    if (x[k>>kshift] & 1 << (k & kmask))
      lostbits |= 2;
    nbits -= n;
    rshift(b,n);
    e = emin;
  }
  if (lostbits) {
    up = 0;
    switch(rounding) {
    case Round_zero:
      break;
    case Round_near:
      if (lostbits & 2
          && (lostbits & 1) | (x[0] & 1))
        up = 1;
      break;
    case Round_up:
      up = 1 - sign;
      break;
    case Round_down:
      up = sign;
    }
    if (up) {
      k = b->wds;
      b = increment(hp, b);
      x = b->x;
      if (denorm) {
#if 0
        if (nbits == Nbits - 1
            && x[nbits >> kshift] & 1 << (nbits & kmask))
          denorm = 0; /* not currently used */
#endif
      }
      else if (b->wds > k
               || ((n = nbits & kmask) !=0
                   && hi0bits(x[k-1]) < 32-n)) {
        rshift(b,1);
        if (++e > Emax)
          goto ovfl;
      }
    }
  }
  if (denorm)
    word0(rvp) = b->wds > 1 ? b->x[1] & ~0x100000 : 0;
  else
    word0(rvp) = (b->x[1] & ~0x100000) | ((e + 0x3ff + 52) << 20);
  word1(rvp) = b->x[0];
  Bfree(hp, b);
}      /* gethex */
#endif /*!NO_HEX_FP}*/

static int
dshift (Bigint *b, int p2)
{
  int rv = hi0bits(b->x[b->wds-1]) - 4;
  if (p2 > 0)
    rv -= p2;
  return rv & kmask;
} /* dshift */

static int
quorem (Bigint *b, Bigint *S)
{
  int n;
  uint32_t *bx, *bxe, q, *sx, *sxe;
  uint64_t borrow, carry, y, ys;

  n = S->wds;
#ifdef DEBUG
  /*debug*/ if (b->wds > n)
    /*debug*/       Bug("oversize b in quorem");
#endif
  if (b->wds < n)
    return 0;
  sx = S->x;
  sxe = sx + --n;
  bx = b->x;
  bxe = bx + n;
  q = *bxe / (*sxe + 1);  /* ensure q <= true quotient */
#ifdef DEBUG
#ifdef NO_STRTOD_BIGCOMP
  /*debug*/ if (q > 9)
#else
    /* An oversized q is possible when quorem is called from bigcomp and */
    /* the input is near, e.g., twice the smallest denormalized number. */
    /*debug*/ if (q > 15)
#endif
      /*debug*/       Bug("oversized quotient in quorem");
#endif
  if (q) {
    borrow = 0;
    carry = 0;
    do {
      ys = *sx++ * (uint64_t)q + carry;
      carry = ys >> 32;
      y = *bx - (ys & FFFFFFFF) - borrow;
      borrow = y >> 32 & (uint32_t)1;
      *bx++ = y & FFFFFFFF;
    }
    while(sx <= sxe);
    if (!*bxe) {
      bx = b->x;
      while(--bxe > bx && !*bxe)
        --n;
      b->wds = n;
    }
  }
  if (cmp(b, S) >= 0) {
    q++;
    borrow = 0;
    carry = 0;
    bx = b->x;
    sx = S->x;
    do {
      ys = *sx++ + carry;
      carry = ys >> 32;
      y = *bx - (ys & FFFFFFFF) - borrow;
      borrow = y >> 32 & (uint32_t)1;
      *bx++ = y & FFFFFFFF;
    }
    while(sx <= sxe);
    bx = b->x;
    bxe = bx + n;
    if (!*bxe) {
      while(--bxe > bx && !*bxe)
        --n;
      b->wds = n;
    }
  }
  return q;
} /* quorem */

static double
sulp (U *x, BCinfo_t *bc)
{
  U u;
  double rv;
  int i;

  rv = ulp(x);
  if (!bc->scale || (i = 2*P + 1 - ((word0(x) & Exp_mask) >> Exp_shift)) <= 0)
    return rv; /* Is there an example where i <= 0 ? */
  word0(&u) = Exp_1 + (i << Exp_shift);
  word1(&u) = 0;
  return rv * u.d;
} /* sulp */


#ifndef NO_STRTOD_BIGCOMP
static void
bigcomp (bigHeap_t *hp, U *rv, const char *s0, BCinfo_t *bc)
{
  Bigint *b, *d;
  int b2, bbits, d2, dd, dig, dsign, i, j, nd, nd0, p2, p5, speccase;

  dsign = bc->dsign;
  nd = bc->nd;
  nd0 = bc->nd0;
  p5 = nd + bc->e0 - 1;
  speccase = 0;
#ifndef Sudden_Underflow
  if (rv->d == 0.) {      /* special case: value near underflow-to-zero */
    /* threshold was rounded to zero */
    b = i2b(hp, 1);
    p2 = Emin - P + 1;
    bbits = 1;
    word0(rv) = (P+2) << Exp_shift;
    i = 0;
#ifdef Honor_FLT_ROUNDS
    if (bc->rounding == 1)
#endif
      {
        speccase = 1;
        --p2;
        dsign = 0;
        goto have_i;
      }
  }
  else
#endif
    b = d2b(hp, rv, &p2, &bbits);
  p2 -= bc->scale;
  /* floor(log2(rv)) == bbits - 1 + p2 */
  /* Check for denormal case. */
  i = P - bbits;
  if (i > (j = P - Emin - 1 + p2)) {
#ifdef Sudden_Underflow
    Bfree(hp, b);
    b = i2b(hp, 1);
    p2 = Emin;
    i = P - 1;
    word0(rv) = (1 + bc->scale) << Exp_shift;
    word1(rv) = 0;
#else
    i = j;
#endif
                }
#ifdef Honor_FLT_ROUNDS
  if (bc->rounding != 1) {
    if (i > 0)
      b = lshift(hp, b, i);
    if (dsign)
      b = increment(b);
  }
  else
#endif
    {
      b = lshift(hp, b, ++i);
      b->x[0] |= 1;
    }
#ifndef Sudden_Underflow
 have_i:
#endif
  p2 -= p5 + i;
  d = i2b(hp, 1);
  /* Arrange for convenient computation of quotients:
   * shift left if necessary so divisor has 4 leading 0 bits.
   */
  if (p5 > 0)
    d = pow5mult(hp, d, p5);
  else if (p5 < 0)
    b = pow5mult(hp, b, -p5);
  if (p2 > 0) {
    b2 = p2;
    d2 = 0;
  }
  else {
    b2 = 0;
    d2 = -p2;
  }
  i = dshift(d, d2);
  if ((b2 += i) > 0)
    b = lshift(hp, b, b2);
  if ((d2 += i) > 0)
    d = lshift(hp, d, d2);

  /* Now b/d = exactly half-way between the two floating-point values */
  /* on either side of the input string.  Compute first digit of b/d. */

  if (!(dig = quorem(b,d))) {
    b = multadd(hp, b, 10, 0);  /* very unlikely */
    dig = quorem(b,d);
  }

  /* Compare b/d with s0 */

  for (i = 0;  i < nd0; ) {
    if ((dd = s0[i++] - '0' - dig))
      goto ret;
    if (!b->x[0] && b->wds == 1) {
      if (i < nd)
        dd = 1;
      goto ret;
    }
    b = multadd(hp, b, 10, 0);
    dig = quorem(b,d);
  }
  for (j = bc->dp1; i++ < nd;) {
    if ((dd = s0[j++] - '0' - dig))
      goto ret;
    if (!b->x[0] && b->wds == 1) {
      if (i < nd)
        dd = 1;
      goto ret;
    }
    b = multadd(hp, b, 10, 0);
    dig = quorem(b,d);
  }
  if (dig > 0 || b->x[0] || b->wds > 1)
    dd = -1;
 ret:
  Bfree(hp,b);
  Bfree(hp,d);
#ifdef Honor_FLT_ROUNDS
  if (bc->rounding != 1) {
    if (dd < 0) {
      if (bc->rounding == 0) {
        if (!dsign)
          goto retlow1;
      }
      else if (dsign)
        goto rethi1;
    }
    else if (dd > 0) {
      if (bc->rounding == 0) {
        if (dsign)
          goto rethi1;
        goto ret1;
      }
      if (!dsign)
        goto rethi1;
      dval(rv) += 2.*sulp(rv,bc);
    }
    else {
      bc->inexact = 0;
      if (dsign)
        goto rethi1;
    }
  }
  else
#endif
    if (speccase) {
      if (dd <= 0)
        rv->d = 0.;
    }
    else if (dd < 0) {
      if (!dsign)     /* does not happen for round-near */
      retlow1:
        dval(rv) -= sulp(rv,bc);
    }
    else if (dd > 0) {
      if (dsign) {
      rethi1:
        dval(rv) += sulp(rv,bc);
      }
    }
    else {
      /* Exact half-way case:  apply round-even rule. */
      if ((j = ((word0(rv) & Exp_mask) >> Exp_shift) - bc->scale) <= 0) {
        i = 1 - j;
        if (i <= 31) {
          if (word1(rv) & (0x1 << i))
            goto odd;
        }
        else if (word0(rv) & (0x1 << (i-32)))
          goto odd;
      }
      else if (word1(rv) & 1) {
      odd:
        if (dsign)
          goto rethi1;
        goto retlow1;
      }
    }

#ifdef Honor_FLT_ROUNDS
 ret1:
#endif
  return;
} /* bigcomp */
#endif /* NO_STRTOD_BIGCOMP */

double DTOA_API DTOA_CALLCONV
strtodLoc (const char *s00, char **se, int *locErrno)
{
  int bb2, bb5, bbe, bd2, bd5, bbbits, bs2, c, e, e1;
  int esign, i, j, k, nd, nd0, nf, nz, nz0, nz1, sign;
  const char *s, *s0, *s1;
  double aadj, aadj1;
  int32_t L;
  U aadj2, adj, rv, rv0;
  uint32_t y, z;
  BCinfo_t bc;
  Bigint *bb=NULL, *bb1, *bd=NULL, *bd0=NULL, *bs=NULL, *delta;
  bigHeap_t heap;
  uint32_t Lsb, Lsb1;
#ifdef SET_INEXACT
  int oldinexact;
#endif
#ifndef NO_STRTOD_BIGCOMP
  int req_bigcomp = 0;
#endif
#ifdef Honor_FLT_ROUNDS /*{*/
#ifdef Trust_FLT_ROUNDS /*{{ only define this if FLT_ROUNDS really works! */
  bc.rounding = Flt_Rounds;
#else /*}{*/
  bc.rounding = 1;
  switch(fegetround()) {
  case FE_TOWARDZERO:   bc.rounding = 0; break;
  case FE_UPWARD:       bc.rounding = 2; break;
  case FE_DOWNWARD:     bc.rounding = 3;
  }
#endif /*}}*/
#endif /*}*/
#ifdef USE_LOCALE
  const char *s2;
#endif

  delta = NULL;                 /* shut up warnings */
  *locErrno = 0;
  sign = nz0 = nz1 = nz = bc.dplen = bc.uflchk = 0;
  dval(&rv) = 0.;
  for (s = s00;  ;  s++)
    switch(*s) {
    case '-':
      sign = 1;
      /* no break */
    case '+':
      if (*++s)
        goto break2;
      /* no break */
    case 0:
      goto ret0;
    case '\t':
    case '\n':
    case '\v':
    case '\f':
    case '\r':
    case ' ':
      continue;
    default:
      goto break2;
    }
 break2:
  if (*s == '0') {
#ifndef NO_HEX_FP /*{*/
    switch(s[1]) {
    case 'x':
    case 'X':
      heapInit (&heap);
#ifdef Honor_FLT_ROUNDS
      gethex(&heap, &s, &rv, bc.rounding, sign);
#else
      gethex(&heap, &s, &rv, 1, sign);
#endif
      goto ret;
    }
#endif /*}*/
    nz0 = 1;
    while (*++s == '0') ;
    if (!*s)
      goto ret;
  }
  s0 = s;
  y = z = 0;
  for (nd = nf = 0; (c = *s) >= '0' && c <= '9'; nd++, s++)
    if (nd < 9)
      y = 10*y + c - '0';
    else if (nd < DBL_DIG + 2)
      z = 10*z + c - '0';
  nd0 = nd;
  bc.dp0 = bc.dp1 = (int) (s - s0);
  for(s1 = s; s1 > s0 && *--s1 == '0'; )
    ++nz1;
#ifdef USE_LOCALE
  s1 = localeconv()->decimal_point;
  if (c == *s1) {
    c = '.';
    if (*++s1) {
      s2 = s;
      for(;;) {
        if (*++s2 != *s1) {
          c = 0;
          break;
        }
        if (!*++s1) {
          s = s2;
          break;
        }
      }
    }
  }
#endif
  if (c == '.') {
    c = *++s;
    bc.dp1 = (int) (s - s0);
    bc.dplen = bc.dp1 - bc.dp0;
    if (!nd) {
      for ( ;  c == '0';  c = *++s)
        nz++;
      if (c > '0' && c <= '9') {
        bc.dp0 = (int) (s0 - s);
        bc.dp1 = bc.dp0 + bc.dplen;
        s0 = s;
        nf += nz;
        nz = 0;
        goto have_dig;
      }
      goto dig_done;
    }
    for ( ;  c >= '0' && c <= '9';  c = *++s) {
    have_dig:
      nz++;
      if (c -= '0') {
        nf += nz;
        for(i = 1; i < nz; i++)
          if (nd++ < 9)
            y *= 10;
          else if (nd <= DBL_DIG + 2)
            z *= 10;
        if (nd++ < 9)
          y = 10*y + c;
        else if (nd <= DBL_DIG + 2)
          z = 10*z + c;
        nz = nz1 = 0;
      }
    }
  }

 dig_done:
  e = 0;
  if (c == 'e' || c == 'E') {
    if (!nd && !nz && !nz0) {
      goto ret0;
    }
    s00 = s;
    esign = 0;
    switch (c = *++s) {
    case '-':
      esign = 1;
    case '+':
      c = *++s;
    }
    if (c >= '0' && c <= '9') {
      while(c == '0')
        c = *++s;
      if (c > '0' && c <= '9') {
        L = c - '0';
        s1 = s;
        while((c = *++s) >= '0' && c <= '9')
          L = 10*L + c - '0';
        if (s - s1 > 8 || L > 19999)
          /* Avoid confusion from exponents
           * so large that e might overflow.
           */
          e = 19999; /* safe for 16 bit ints */
        else
          e = (int)L;
        if (esign)
          e = -e;
      }
      else
        e = 0;
    }
    else
      s = s00;
  }
  if (!nd) {
    if (!nz && !nz0) {
#ifdef INFNAN_CHECK
      /* Check for Nan and Infinity */
      if (!bc.dplen)
        switch(c) {
        case 'i':
        case 'I':
          if (match(&s,"nf")) {
            --s;
            if (!match(&s,"inity"))
              ++s;
            word0(&rv) = 0x7ff00000;
            word1(&rv) = 0;
            goto ret;
          }
          break;
        case 'n':
        case 'N':
          if (match(&s, "an")) {
            word0(&rv) = NAN_WORD0;
            word1(&rv) = NAN_WORD1;
#ifndef No_Hex_NaN
            if (*s == '(') /*)*/
              hexnan(&rv, &s);
#endif
            goto ret;
          }
        }
#endif /* INFNAN_CHECK */
    ret0:
      s = s00;
      sign = 0;
    }
    goto ret;
  }
  bc.e0 = e1 = e -= nf;

  /* Now we have nd0 digits, starting at s0, followed by a
   * decimal point, followed by nd-nd0 digits.  The number we're
   * after is the integer represented by those digits times
   * 10**e */

  if (!nd0)
    nd0 = nd;
  k = nd < DBL_DIG + 2 ? nd : DBL_DIG + 2;
  dval(&rv) = y;
  if (k > 9) {
#ifdef SET_INEXACT
    if (k > DBL_DIG)
      oldinexact = get_inexact();
#endif
    dval(&rv) = tens[k - 9] * dval(&rv) + z;
  }
  bd0 = 0;
  if (nd <= DBL_DIG
#ifndef Honor_FLT_ROUNDS
      && Flt_Rounds == 1
#endif
      ) {
    if (!e)
      goto ret;
#ifndef ROUND_BIASED_without_Round_Up
    if (e > 0) {
      if (e <= Ten_pmax) {
#ifdef Honor_FLT_ROUNDS
        /* round correctly FLT_ROUNDS = 2 or 3 */
        if (sign) {
          rv.d = -rv.d;
          sign = 0;
        }
#endif
        /* rv = */ rounded_product(dval(&rv), tens[e]);
        goto ret;
      }
      i = DBL_DIG - nd;
      if (e <= Ten_pmax + i) {
        /* A fancier test would sometimes let us do
         * this for larger i values.
         */
#ifdef Honor_FLT_ROUNDS
        /* round correctly FLT_ROUNDS = 2 or 3 */
        if (sign) {
          rv.d = -rv.d;
          sign = 0;
        }
#endif
        e -= i;
        dval(&rv) *= tens[i];
        /* rv = */ rounded_product(dval(&rv), tens[e]);
        goto ret;
      }
    }
    else if (e >= -Ten_pmax) {
#ifdef Honor_FLT_ROUNDS
      /* round correctly FLT_ROUNDS = 2 or 3 */
      if (sign) {
        rv.d = -rv.d;
        sign = 0;
      }
#endif
      /* rv = */ rounded_quotient(dval(&rv), tens[-e]);
      goto ret;
    }
#endif /* ROUND_BIASED_without_Round_Up */
  }
  e1 += nd - k;

#ifdef SET_INEXACT
  bc.inexact = 1;
  if (k <= DBL_DIG)
    oldinexact = get_inexact();
#endif
  bc.scale = 0;
#ifdef Honor_FLT_ROUNDS
  if (bc.rounding >= 2) {
    if (sign)
      bc.rounding = bc.rounding == 2 ? 0 : 2;
    else
      if (bc.rounding != 2)
        bc.rounding = 0;
  }
#endif

  /* Get starting approximation = rv * 10**e1 */

  heapInit (&heap);
  if (e1 > 0) {
    if ((i = e1 & 15))
      dval(&rv) *= tens[i];
    if (e1 &= ~15) {
      if (e1 > DBL_MAX_10_EXP) {
 ovfl:
        /* Can't trust HUGE_VAL */
#ifdef Honor_FLT_ROUNDS
        switch(bc.rounding) {
        case 0: /* toward 0 */
        case 3: /* toward -infinity */
          word0(&rv) = Big0;
          word1(&rv) = Big1;
          break;
        default:
          word0(&rv) = Exp_mask;
          word1(&rv) = 0;
        }
#else /*Honor_FLT_ROUNDS*/
        word0(&rv) = Exp_mask;
        word1(&rv) = 0;
#endif /*Honor_FLT_ROUNDS*/
#ifdef SET_INEXACT
        /* set overflow bit */
        dval(&rv0) = 1e300;
        dval(&rv0) *= dval(&rv0);
#endif
      range_err:
        if (bd0) {
          Bfree(&heap,bb);
          Bfree(&heap,bd);
          Bfree(&heap,bs);
          Bfree(&heap,bd0);
          Bfree(&heap,delta);
        }
        *locErrno = ERANGE;
        goto ret;
      }
      e1 >>= 4;
      for(j = 0; e1 > 1; j++, e1 >>= 1)
        if (e1 & 1)
          dval(&rv) *= bigtens[j];
      /* The last multiplication could overflow. */
      word0(&rv) -= P*Exp_msk1;
      dval(&rv) *= bigtens[j];
      if ((z = word0(&rv) & Exp_mask)
          > Exp_msk1*(DBL_MAX_EXP+Bias-P))
        goto ovfl;
      if (z > Exp_msk1*(DBL_MAX_EXP+Bias-1-P)) {
        /* set to largest number */
        /* (Can't trust DBL_MAX) */
        word0(&rv) = Big0;
        word1(&rv) = Big1;
      }
      else
        word0(&rv) += P*Exp_msk1;
    }
  }
  else if (e1 < 0) {
    e1 = -e1;
    if ((i = e1 & 15))
      dval(&rv) /= tens[i];
    if (e1 >>= 4) {
      if (e1 >= 1 << n_bigtens)
        goto undfl;
      if (e1 & Scale_Bit)
        bc.scale = 2*P;
      for(j = 0; e1 > 0; j++, e1 >>= 1)
        if (e1 & 1)
          dval(&rv) *= tinytens[j];
      if (bc.scale && (j = 2*P + 1 - ((word0(&rv) & Exp_mask)
                                      >> Exp_shift)) > 0) {
        /* scaled rv is denormal; clear j low bits */
        if (j >= 32) {
          if (j > 54)
            goto undfl;
          word1(&rv) = 0;
          if (j >= 53)
            word0(&rv) = (P+2)*Exp_msk1;
          else
            word0(&rv) &= 0xffffffff << (j-32);
        }
        else
          word1(&rv) &= 0xffffffff << j;
      }
        if (!dval(&rv)) {
 undfl:
          dval(&rv) = 0.;
          goto range_err;
        }
    }
  }

  /* Now the hard part -- adjusting rv to the correct value.*/

  /* Put digits into bd: true value = bd * 10^e */

  bc.nd = nd - nz1;
#ifndef NO_STRTOD_BIGCOMP
  bc.nd0 = nd0;   /* Only needed if nd > strtod_diglim, but done here */
                  /* to silence an erroneous warning about bc.nd0 */
                  /* possibly not being initialized. */
  if (nd > strtod_diglim) {
    /* ASSERT(strtod_diglim >= 18); 18 == one more than the */
    /* minimum number of decimal digits to distinguish double values */
    /* in IEEE arithmetic. */
    i = j = 18;
    if (i > nd0)
      j += bc.dplen;
    for (;;) {
      if (--j < bc.dp1 && j >= bc.dp0)
        j = bc.dp0 - 1;
      if (s0[j] != '0')
        break;
      --i;
    }
    e += nd - i;
    nd = i;
    if (nd0 > nd)
      nd0 = nd;
    if (nd < 9) { /* must recompute y */
      y = 0;
      for(i = 0; i < nd0; ++i)
        y = 10*y + s0[i] - '0';
      for (j = bc.dp1; i < nd; ++i)
        y = 10*y + s0[j++] - '0';
    }
  }
#endif

#if 1 == DTOA_USE_ND_BOUND
# if DTOA_ND_BOUND < 18
#  error "DTOA_ND_BOUND must be at least 18"
/* 18 == one more than the minimum number of decimal digits required
 * to distinguish double values in IEEE arithmetic */
# endif
  if (nd > DTOA_ND_BOUND) {
    e += nd - DTOA_ND_BOUND;
    nd = DTOA_ND_BOUND;
    if (nd0 > nd)
      nd0 = nd;
  }
#endif

  bd0 = s2b(&heap, s0, nd0, nd, y, bc.dplen);

  for (;;) {
    bd = Balloc(&heap, bd0->k);
    BCOPY(bd, bd0);
    bb = d2b(&heap, &rv, &bbe, &bbbits); /* rv = bb * 2^bbe */
    bs = i2b(&heap, 1);

    if (e >= 0) {
      bb2 = bb5 = 0;
      bd2 = bd5 = e;
    }
    else {
      bb2 = bb5 = -e;
      bd2 = bd5 = 0;
    }
    if (bbe >= 0)
      bb2 += bbe;
    else
      bd2 -= bbe;
    bs2 = bb2;
#ifdef Honor_FLT_ROUNDS
    if (bc.rounding != 1)
      bs2++;
#endif
    Lsb = LSB;
    Lsb1 = 0;
    j = bbe - bc.scale;
    i = j + bbbits - 1;     /* logb(rv) */
    j = P + 1 - bbbits;
    if (i < Emin) { /* denormal */
      i = Emin - i;
      j -= i;
      if (i < 32)
        Lsb <<= i;
      else if (i < 52)
        Lsb1 = Lsb << (i-32);
      else
        Lsb1 = Exp_mask;
    }
    bb2 += j;
    bd2 += j;
    bd2 += bc.scale;
    i = bb2 < bd2 ? bb2 : bd2;
    if (i > bs2)
      i = bs2;
    if (i > 0) {
      bb2 -= i;
      bd2 -= i;
      bs2 -= i;
    }
    if (bb5 > 0) {
      bs = pow5mult(&heap, bs, bb5);
      bb1 = mult(&heap, bs, bb);
      Bfree(&heap, bb);
      bb = bb1;
    }
    if (bb2 > 0)
      bb = lshift(&heap, bb, bb2);
    if (bd5 > 0)
      bd = pow5mult(&heap, bd, bd5);
    if (bd2 > 0)
      bd = lshift(&heap, bd, bd2);
    if (bs2 > 0)
      bs = lshift(&heap, bs, bs2);
    delta = diff(&heap, bb, bd);
    bc.dsign = delta->sign;
    delta->sign = 0;
    i = cmp(delta, bs);
#ifndef NO_STRTOD_BIGCOMP /*{*/
    if (bc.nd > nd && i <= 0) {
      if (bc.dsign) {
        /* Must use bigcomp(). */
        req_bigcomp = 1;
        break;
      }
#ifdef Honor_FLT_ROUNDS
      if (bc.rounding != 1) {
        if (i < 0) {
          req_bigcomp = 1;
          break;
        }
      }
      else
#endif
        i = -1; /* Discarded digits make delta smaller. */
    }
#endif /*}*/
#ifdef Honor_FLT_ROUNDS /*{*/
    if (bc.rounding != 1) {
      if (i < 0) {
        /* Error is less than an ulp */
        if (!delta->x[0] && delta->wds <= 1) {
          /* exact */
#ifdef SET_INEXACT
          bc.inexact = 0;
#endif
          break;
        }
        if (bc.rounding) {
          if (bc.dsign) {
            adj.d = 1.;
            goto apply_adj;
          }
        }
        else if (!bc.dsign) {
          adj.d = -1.;
          if (!word1(&rv)
              && !(word0(&rv) & Frac_mask)) {
            y = word0(&rv) & Exp_mask;
            if (!bc.scale || y > 2*P*Exp_msk1) {
              delta = lshift(&heap,delta,Log2P);
              if (cmp(delta, bs) <= 0)
                adj.d = -0.5;
            }
          }
 apply_adj:
          if (bc.scale && (y = word0(&rv) & Exp_mask)
              <= 2*P*Exp_msk1)
            word0(&adj) += (2*P+1)*Exp_msk1 - y;
          dval(&rv) += adj.d*ulp(&rv);
        }
        break;
      }
      adj.d = ratio(delta, bs);
      if (adj.d < 1.)
        adj.d = 1.;
      if (adj.d <= 0x7ffffffe) {
        /* adj = rounding ? ceil(adj) : floor(adj); */
        y = adj.d;
        if (y != adj.d) {
          if (!((bc.rounding>>1) ^ bc.dsign))
            y++;
          adj.d = y;
        }
      }
      if (bc.scale && (y = word0(&rv) & Exp_mask) <= 2*P*Exp_msk1)
        word0(&adj) += (2*P+1)*Exp_msk1 - y;
      adj.d *= ulp(&rv);
      if (bc.dsign) {
        if (word0(&rv) == Big0 && word1(&rv) == Big1)
          goto ovfl;
        dval(&rv) += adj.d;
      }
      else
        dval(&rv) -= adj.d;
      goto cont;
    }
#endif /*}Honor_FLT_ROUNDS*/

    if (i < 0) {
      /* Error is less than half an ulp -- check for
       * special case of mantissa a power of two.
       */
      if (bc.dsign || word1(&rv) || word0(&rv) & Bndry_mask
          || (word0(&rv) & Exp_mask) <= (2*P+1)*Exp_msk1
          ) {
#ifdef SET_INEXACT
        if (!delta->x[0] && delta->wds <= 1)
          bc.inexact = 0;
#endif
        break;
      }
      if (!delta->x[0] && delta->wds <= 1) {
        /* exact result */
#ifdef SET_INEXACT
        bc.inexact = 0;
#endif
        break;
      }
      delta = lshift(&heap,delta,Log2P);
      if (cmp(delta, bs) > 0)
        goto drop_down;
      break;
    }
    if (i == 0) {
      /* exactly half-way between */
      if (bc.dsign) {
        if ((word0(&rv) & Bndry_mask1) == Bndry_mask1
            &&  word1(&rv) == (
                               (bc.scale && (y = word0(&rv) & Exp_mask) <= 2*P*Exp_msk1)
                               ? (0xffffffff & (0xffffffff << (2*P+1-(y>>Exp_shift)))) :
                               0xffffffff)) {
          /*boundary case -- increment exponent*/
          if (word0(&rv) == Big0 && word1(&rv) == Big1)
            goto ovfl;
          word0(&rv) = (word0(&rv) & Exp_mask)
            + Exp_msk1;
          word1(&rv) = 0;
          bc.dsign = 0;
          break;
        }
      }
      else if (!(word0(&rv) & Bndry_mask) && !word1(&rv)) {
      drop_down:
        /* boundary case -- decrement exponent */
#ifdef Sudden_Underflow /*{{*/
        L = word0(&rv) & Exp_mask;
        if (L <= (bc.scale ? (2*P+1)*Exp_msk1 : Exp_msk1)) {
          if (bc.nd >nd) {
            bc.uflchk = 1;
            break;
          }
          goto undfl;
        }
        L -= Exp_msk1;
#else /*Sudden_Underflow}{*/
        if (bc.scale) {
          L = word0(&rv) & Exp_mask;
          if (L <= (2*P+1)*Exp_msk1) {
            if (L > (P+2)*Exp_msk1)
              /* round even ==> */
              /* accept rv */
              break;
            /* rv = smallest denormal */
            if (bc.nd >nd) {
              bc.uflchk = 1;
              break;
            }
            goto undfl;
          }
        }
        L = (word0(&rv) & Exp_mask) - Exp_msk1;
#endif /*Sudden_Underflow}}*/
        word0(&rv) = L | Bndry_mask1;
        word1(&rv) = 0xffffffff;
#ifndef NO_STRTOD_BIGCOMP
        if (bc.nd > nd)
          goto cont;
#endif
        break;
      }
#ifndef ROUND_BIASED
      if (Lsb1) {
        if (!(word0(&rv) & Lsb1))
          break;
      }
      else if (!(word1(&rv) & Lsb))
        break;
#endif
      if (bc.dsign)
        dval(&rv) += sulp(&rv, &bc);
#ifndef ROUND_BIASED
      else {
        dval(&rv) -= sulp(&rv, &bc);
#ifndef Sudden_Underflow
        if (!dval(&rv)) {
          if (bc.nd >nd) {
            bc.uflchk = 1;
            break;
          }
          goto undfl;
        }
#endif
      }
      bc.dsign = 1 - bc.dsign;
#endif
      break;
    }
    if ((aadj = ratio(delta, bs)) <= 2.) {
      if (bc.dsign)
        aadj = aadj1 = 1.;
      else if (word1(&rv) || word0(&rv) & Bndry_mask) {
#ifndef Sudden_Underflow
        if (word1(&rv) == Tiny1 && !word0(&rv)) {
          if (bc.nd >nd) {
            bc.uflchk = 1;
            break;
          }
          goto undfl;
        }
#endif
        aadj = 1.;
        aadj1 = -1.;
      }
      else {
        /* special case -- power of FLT_RADIX to be */
        /* rounded down... */

        if (aadj < 2./FLT_RADIX)
          aadj = 1./FLT_RADIX;
        else
          aadj *= 0.5;
        aadj1 = -aadj;
      }
    }
    else {
      aadj *= 0.5;
      aadj1 = bc.dsign ? aadj : -aadj;
#ifdef Check_FLT_ROUNDS
      switch(bc.rounding) {
      case 2: /* towards +infinity */
        aadj1 -= 0.5;
        break;
      case 0: /* towards 0 */
      case 3: /* towards -infinity */
        aadj1 += 0.5;
      }
#else
      if (Flt_Rounds == 0)
        aadj1 += 0.5;
#endif /*Check_FLT_ROUNDS*/
    }
    y = word0(&rv) & Exp_mask;

    /* Check for overflow */

    if (y == Exp_msk1*(DBL_MAX_EXP+Bias-1)) {
      dval(&rv0) = dval(&rv);
      word0(&rv) -= P*Exp_msk1;
      adj.d = aadj1 * ulp(&rv);
      dval(&rv) += adj.d;
      if ((word0(&rv) & Exp_mask) >=
          Exp_msk1*(DBL_MAX_EXP+Bias-P)) {
        if (word0(&rv0) == Big0 && word1(&rv0) == Big1)
          goto ovfl;
        word0(&rv) = Big0;
        word1(&rv) = Big1;
        goto cont;
      }
      else
        word0(&rv) += P*Exp_msk1;
    }
    else {
      if (bc.scale && y <= 2*P*Exp_msk1) {
        if (aadj <= 0x7fffffff) {
          if ((z = (uint32_t)aadj) <= 0)
            z = 1;
          aadj = z;
          aadj1 = bc.dsign ? aadj : -aadj;
        }
        dval(&aadj2) = aadj1;
        word0(&aadj2) += (2*P+1)*Exp_msk1 - y;
        aadj1 = dval(&aadj2);
        adj.d = aadj1 * ulp(&rv);
        dval(&rv) += adj.d;
        if (rv.d == 0.)
#ifdef NO_STRTOD_BIGCOMP
          goto undfl;
#else
        {
          req_bigcomp = 1;
          break;
        }
#endif
      }
      else {
        adj.d = aadj1 * ulp(&rv);
        dval(&rv) += adj.d;
      }
    }
    z = word0(&rv) & Exp_mask;
#ifndef SET_INEXACT
    if (bc.nd == nd) {
      if (!bc.scale)
        if (y == z) {
          /* Can we stop now? */
          L = (int32_t)aadj;
          aadj -= L;
          /* The tolerances below are conservative. */
          if (bc.dsign || word1(&rv) || word0(&rv) & Bndry_mask) {
            if (aadj < .4999999 || aadj > .5000001)
              break;
          }
          else if (aadj < .4999999/FLT_RADIX)
            break;
        }
    }
#endif
  cont:
    Bfree(&heap, bb);
    Bfree(&heap, bd);
    Bfree(&heap, bs);
    Bfree(&heap, delta);
  }
  Bfree(&heap, bb);
  Bfree(&heap, bd);
  Bfree(&heap, bs);
  Bfree(&heap, bd0);
  Bfree(&heap, delta);
#ifndef NO_STRTOD_BIGCOMP
  if (req_bigcomp) {
    bd0 = 0;
    bc.e0 += nz1;
    bigcomp(&heap, &rv, s0, &bc);
    y = word0(&rv) & Exp_mask;
    if (y == Exp_mask)
      goto ovfl;
    if (y == 0 && rv.d == 0.)
      goto undfl;
  }
#endif
#ifdef SET_INEXACT
  if (bc.inexact) {
    if (!oldinexact) {
      word0(&rv0) = Exp_1 + (70 << Exp_shift);
      word1(&rv0) = 0;
      dval(&rv0) += 1.;
    }
  }
  else if (!oldinexact)
    clear_inexact();
#endif
  if (bc.scale) {
    word0(&rv0) = Exp_1 - 2*P*Exp_msk1;
    word1(&rv0) = 0;
    dval(&rv) *= dval(&rv0);
    /* try to avoid the bug of testing an 8087 register value */
    if (!(word0(&rv) & Exp_mask))
      *locErrno = ERANGE;
  }
#ifdef SET_INEXACT
  if (bc.inexact && !(word0(&rv) & Exp_mask)) {
    /* set underflow bit */
    dval(&rv0) = 1e-300;
    dval(&rv0) *= dval(&rv0);
  }
#endif
 ret:
  if (se)
    *se = (char *)s;
  return sign ? -dval(&rv) : dval(&rv);
} /* strtodLoc */

static char *
noReturnVal (const char *s, char *buf, size_t bufSiz, char **rve)
{
  size_t n;

  n = strlen(s);
  if (n >= bufSiz) {
    if (rve)
      *rve = NULL;
    return NULL;
  }
  strcpy (buf, s);
  if (rve)
    *rve = buf + n;
  return buf;
} /* noReturnVal */


/* dtoa for IEEE arithmetic (dmg): convert double to ASCII string.
 *
 * Inspired by "How to Print Floating-Point Numbers Accurately" by
 * Guy L. Steele, Jr. and Jon L. White [Proc. ACM SIGPLAN '90, pp. 112-126].
 *
 * Modifications:
 *      1. Rather than iterating, we use a simple numeric overestimate
 *         to determine k = floor(log10(d)).  We scale relevant
 *         quantities using O(log2(k)) rather than O(k) multiplications.
 *      2. For some modes > 2 (corresponding to ecvt and fcvt), we don't
 *         try to generate digits strictly left to right.  Instead, we
 *         compute with fewer bits and propagate the carry if necessary
 *         when rounding the final digit up.  This is often faster.
 *      3. Under the assumption that input will be rounded nearest,
 *         mode 0 renders 1e23 as 1e23 rather than 9.999999999999999e22.
 *         That is, we allow equality in stopping tests when the
 *         round-nearest rule will give the same floating-point value
 *         as would satisfaction of the stopping test with strict
 *         inequality.
 *      4. We remove common factors of powers of 2 from relevant
 *         quantities.
 *      5. When converting floating-point integers less than 1e16,
 *         we use floating-point arithmetic rather than resorting
 *         to multiple-precision integers.
 *      6. When asked to produce fewer than 15 digits, we first try
 *         to get by with floating-point arithmetic; we resort to
 *         multiple-precision integer arithmetic only if we cannot
 *         guarantee that the floating-point calculation has given
 *         the correctly rounded result.  For k requested digits and
 *         "uniformly" distributed input, the probability is
 *         something like 10^(k-15) that we must resort to the Long
 *         calculation.
 */

char DTOA_API * DTOA_CALLCONV
dtoaLoc (double dd, int mode, int ndigits,
         char buf[], size_t bufSiz, int *decpt, int *sign, char **rve)
{
 /*     Arguments ndigits, decpt, sign are similar to those
        of ecvt and fcvt; trailing zeros are suppressed from
        the returned string.  If not null, *rve is set to point
        to the end of the return value.  If d is +-Infinity or NaN,
        then *decpt is set to 9999.

        mode:
                0 ==> shortest string that yields d when read in
                        and rounded to nearest.
                1 ==> like 0, but with Steele & White stopping rule;
                        e.g. with IEEE P754 arithmetic , mode 0 gives
                        1e23 whereas mode 1 gives 9.999999999999999e22.
                2 ==> max(1,ndigits) significant digits.  This gives a
                        return value similar to that of ecvt, except
                        that trailing zeros are suppressed.
                3 ==> through ndigits past the decimal point.  This
                        gives a return value similar to that from fcvt,
                        except that trailing zeros are suppressed, and
                        ndigits can be negative.
                4,5 ==> similar to 2 and 3, respectively, but (in
                        round-nearest mode) with the tests of mode 0 to
                        possibly return a shorter string that rounds to d.
                        With IEEE arithmetic and compilation with
                        -DHonor_FLT_ROUNDS, modes 4 and 5 behave the same
                        as modes 2 and 3 when FLT_ROUNDS != 1.
                6-9 ==> Debugging modes similar to mode - 4:  don't try
                        fast floating-point estimate (if applicable).

                Values of mode other than 0-9 are treated as mode 0.

                Sufficient space is allocated to the return value
                to hold the suppressed trailing zeros.
        */

  int bbits, b2, b5, be, dig, i, ieps, ilim, ilim0, ilim1,
    j, j1, k, k0, k_check, leftright, m2, m5, s2, s5,
    spec_case, try_quick;
  int32_t L;
#ifndef Sudden_Underflow
  int denorm;
  uint32_t x;
#endif
  Bigint *b, *b1, *delta, *mlo, *mhi, *S;
  bigHeap_t heap;
  U d2, eps, u;
  double ds;
  char *s, *s0;
#ifndef No_leftright
  U eps1;
#endif
#ifdef SET_INEXACT
  int inexact, oldinexact;
#endif
#ifdef Honor_FLT_ROUNDS /*{*/
  int Rounding;
#ifdef Trust_FLT_ROUNDS /*{{ only define this if FLT_ROUNDS really works! */
  Rounding = Flt_Rounds;
#else /*}{*/
  Rounding = 1;
  switch(fegetround()) {
   case FE_TOWARDZERO:   Rounding = 0; break;
   case FE_UPWARD:       Rounding = 2; break;
   case FE_DOWNWARD:     Rounding = 3;
  }
#endif /*}}*/
#endif /*}*/

  j1 = 0;
  heapInit (&heap);
  u.d = dd;
  if (word0(&u) & Sign_bit) {
    /* set sign for everything, including 0's and NaNs */
    *sign = 1;
    word0(&u) &= ~Sign_bit; /* clear sign bit */
  }
  else
    *sign = 0;

  if ((word0(&u) & Exp_mask) == Exp_mask) {
    /* Infinity or NaN */
    *decpt = 9999;
    if (!word1(&u) && !(word0(&u) & 0xfffff)) {
      if (*sign)
        return noReturnVal ("-Inf", buf, bufSiz, rve);
      else
        return noReturnVal ("+Inf", buf, bufSiz, rve);
    }
    return noReturnVal ("Nan", buf, bufSiz, rve);
  }
  if (!dval(&u)) {
    *decpt = 1;
    return noReturnVal ("0", buf, bufSiz, rve);
  }

#ifdef SET_INEXACT
  try_quick = oldinexact = get_inexact();
  inexact = 1;
#endif
#ifdef Honor_FLT_ROUNDS
  if (Rounding >= 2) {
    if (*sign)
      Rounding = Rounding == 2 ? 0 : 2;
    else if (Rounding != 2)
      Rounding = 0;
  }
#endif

  b = d2b(&heap, &u, &be, &bbits);
#ifdef Sudden_Underflow
  i = (int)(word0(&u) >> Exp_shift1 & (Exp_mask>>Exp_shift1));
#else
  if ((i = (int)(word0(&u) >> Exp_shift1 & (Exp_mask>>Exp_shift1)))) {
#endif
    dval(&d2) = dval(&u);
    word0(&d2) &= Frac_mask1;
    word0(&d2) |= Exp_11;

    /* log(x)       ~=~ log(1.5) + (x-1.5)/1.5
     * log10(x)      =  log(x) / log(10)
     *              ~=~ log(1.5)/log(10) + (x-1.5)/(1.5*log(10))
     * log10(d) = (i-Bias)*log(2)/log(10) + log10(d2)
     *
     * This suggests computing an approximation k to log10(d) by
     *
     * k = (i - Bias)*0.301029995663981
     *      + ( (d2-1.5)*0.289529654602168 + 0.176091259055681 );
     *
     * We want k to be too large rather than too small.
     * The error in the first-order Taylor series approximation
     * is in our favor, so we just round up the constant enough
     * to compensate for any error in the multiplication of
     * (i - Bias) by 0.301029995663981; since |i - Bias| <= 1077,
     * and 1077 * 0.30103 * 2^-52 ~=~ 7.2e-14,
     * adding 1e-13 to the constant term more than suffices.
     * Hence we adjust the constant term to 0.1760912590558.
     * (We could get a more accurate k by invoking log10,
     *  but this is probably not worthwhile.)
     */

    i -= Bias;
#ifndef Sudden_Underflow
    denorm = 0;
  }
  else {
    /* d is denormalized */

    i = bbits + be + (Bias + (P-1) - 1);
    x = i > 32  ? word0(&u) << (64 - i) | word1(&u) >> (i - 32)
      : word1(&u) << (32 - i);
    dval(&d2) = x;
    word0(&d2) -= 31*Exp_msk1; /* adjust exponent */
    i -= (Bias + (P-1) - 1) + 1;
    denorm = 1;
  }
#endif
  ds = (dval(&d2)-1.5)*0.289529654602168 + 0.1760912590558 + i*0.301029995663981;
  k = (int)ds;
  if (ds < 0. && ds != k)
    k--;    /* want k = floor(ds) */
  k_check = 1;
  if (k >= 0 && k <= Ten_pmax) {
    if (dval(&u) < tens[k])
      k--;
    k_check = 0;
  }
  j = bbits - i - 1;
  if (j >= 0) {
    b2 = 0;
    s2 = j;
  }
  else {
    b2 = -j;
    s2 = 0;
  }
  if (k >= 0) {
    b5 = 0;
    s5 = k;
    s2 += k;
  }
  else {
    b2 -= k;
    b5 = -k;
    s5 = 0;
  }
  if (mode < 0 || mode > 9)
    mode = 0;

#ifndef SET_INEXACT
#ifdef Check_FLT_ROUNDS
  try_quick = Rounding == 1;
#else
  try_quick = 1;
#endif
#endif /*SET_INEXACT*/

  if (mode > 5) {
    mode -= 4;
    try_quick = 0;
  }
  leftright = 1;
  ilim = ilim1 = -1;      /* Values for cases 0 and 1; done here to */
                          /* silence erroneous "gcc -Wall" warning. */
  switch(mode) {
  case 0:
  case 1:
    i = 18;
    ndigits = 0;
    break;
  case 2:
    leftright = 0;
    /* no break */
  case 4:
    if (ndigits <= 0)
      ndigits = 1;
    ilim = ilim1 = i = ndigits;
    break;
  case 3:
    leftright = 0;
    /* no break */
  case 5:
    i = ndigits + k + 1;
    ilim = i;
    ilim1 = i - 1;
    if (i <= 0)
      i = 1;
  }
  if ((size_t)i >= bufSiz) {
    if (rve)
      *rve = NULL;
    return NULL;
  }
  s = s0 = buf;

#ifdef Honor_FLT_ROUNDS
  if (mode > 1 && Rounding != 1)
    leftright = 0;
#endif

  if (ilim >= 0 && ilim <= Quick_max && try_quick) {

    /* Try to get by with floating-point arithmetic. */

    i = 0;
    dval(&d2) = dval(&u);
    k0 = k;
    ilim0 = ilim;
    ieps = 2; /* conservative */
    if (k > 0) {
      ds = tens[k&0xf];
      j = k >> 4;
      if (j & Bletch) {
        /* prevent overflows */
        j &= Bletch - 1;
        dval(&u) /= bigtens[n_bigtens-1];
        ieps++;
      }
      for ( ;  j;  j >>= 1, i++)
        if (j & 1) {
          ieps++;
          ds *= bigtens[i];
        }
      dval(&u) /= ds;
    }
    else if ((j1 = -k)) {
      dval(&u) *= tens[j1 & 0xf];
      for(j = j1 >> 4; j; j >>= 1, i++)
        if (j & 1) {
          ieps++;
          dval(&u) *= bigtens[i];
        }
    }
    if (k_check && dval(&u) < 1. && ilim > 0) {
      if (ilim1 <= 0)
        goto fast_failed;
      ilim = ilim1;
      k--;
      dval(&u) *= 10.;
      ieps++;
    }
    dval(&eps) = ieps*dval(&u) + 7.;
    word0(&eps) -= (P-1)*Exp_msk1;
    if (ilim == 0) {
      S = mhi = 0;
      dval(&u) -= 5.;
      if (dval(&u) > dval(&eps))
        goto one_digit;
      if (dval(&u) < -dval(&eps))
        goto no_digits;
      goto fast_failed;
                        }
#ifndef No_leftright
    if (leftright) {
      /* Use Steele & White method of only
       * generating digits needed.
       */
      dval(&eps) = 0.5/tens[ilim-1] - dval(&eps);
      if (k0 < 0 && j1 >= 307) {
        eps1.d = 1.01e256; /* 1.01 allows roundoff in the next few lines */
        word0(&eps1) -= Exp_msk1 * (Bias+P-1);
        dval(&eps1) *= tens[j1 & 0xf];
        for(i = 0, j = (j1-256) >> 4; j; j >>= 1, i++)
          if (j & 1)
            dval(&eps1) *= bigtens[i];
        if (eps.d < eps1.d)
          eps.d = eps1.d;
      }
      for (i = 0; ; ) {
        L = (int32_t) dval(&u);
        dval(&u) -= L;
        *s++ = '0' + (int)L;
        if (1. - dval(&u) < dval(&eps))
          goto bump_up;
        if (dval(&u) < dval(&eps))
          goto ret1;
        if (++i >= ilim)
          break;
        dval(&eps) *= 10.;
        dval(&u) *= 10.;
      }
    }
    else {
#endif
      /* Generate ilim digits, then fix them up. */
      dval(&eps) *= tens[ilim-1];
      for(i = 1;; i++, dval(&u) *= 10.) {
        L = (int32_t)(dval(&u));
        if (!(dval(&u) -= L))
          ilim = i;
        *s++ = '0' + (int)L;
        if (i == ilim) {
          if (dval(&u) > 0.5 + dval(&eps))
            goto bump_up;
          else if (dval(&u) < 0.5 - dval(&eps)) {
            while(*--s == '0');
            s++;
            goto ret1;
          }
          break;
        }
      }
#ifndef No_leftright
    }
#endif
  fast_failed:
    s = s0;
    dval(&u) = dval(&d2);
    k = k0;
    ilim = ilim0;
  }

  /* Do we have a "small" integer? */

  if (be >= 0 && k <= Int_max) {
    /* Yes. */
    ds = tens[k];
    if (ndigits < 0 && ilim <= 0) {
      S = mhi = 0;
      if (ilim < 0 || dval(&u) <= 5*ds)
        goto no_digits;
      goto one_digit;
    }
    for (i = 1;  ;  i++, dval(&u) *= 10.) {
      L = (int32_t)(dval(&u) / ds);
      dval(&u) -= L*ds;
#ifdef Check_FLT_ROUNDS
      /* If FLT_ROUNDS == 2, L will usually be high by 1 */
      if (dval(&u) < 0) {
        L--;
        dval(&u) += ds;
      }
#endif
      *s++ = '0' + (int)L;
      if (!dval(&u)) {
#ifdef SET_INEXACT
        inexact = 0;
#endif
        break;
      }
      if (i == ilim) {
#ifdef Honor_FLT_ROUNDS
        if (mode > 1)
          switch(Rounding) {
          case 0: goto ret1;
          case 2: goto bump_up;
          }
#endif
        dval(&u) += dval(&u);
#ifdef ROUND_BIASED
        if (dval(&u) >= ds)
#else
        if (dval(&u) > ds || (dval(&u) == ds && L & 1))
#endif
          {
          bump_up:
            while(*--s == '9')
              if (s == s0) {
                k++;
                *s = '0';
                break;
              }
            ++*s++;
          }
        break;
      }
    }
    goto ret1;
  }

  m2 = b2;
  m5 = b5;
  mhi = mlo = 0;
  if (leftright) {
    i =
#ifndef Sudden_Underflow
      denorm ? be + (Bias + (P-1) - 1 + 1) :
#endif
      1 + P - bbits;
    b2 += i;
    s2 += i;
    mhi = i2b(&heap, 1);
  }
  if (m2 > 0 && s2 > 0) {
    i = m2 < s2 ? m2 : s2;
    b2 -= i;
    m2 -= i;
    s2 -= i;
  }
  if (b5 > 0) {
    if (leftright) {
      if (m5 > 0) {
        mhi = pow5mult(&heap, mhi, m5);
        b1 = mult(&heap, mhi, b);
        Bfree(&heap, b);
        b = b1;
      }
      if ((j = b5 - m5))
        b = pow5mult(&heap, b, j);
    }
    else
      b = pow5mult(&heap, b, b5);
  }
  S = i2b(&heap, 1);
  if (s5 > 0)
    S = pow5mult(&heap, S, s5);

  /* Check for special case that d is a normalized power of 2. */

  spec_case = 0;
  if ((mode < 2 || leftright)
#ifdef Honor_FLT_ROUNDS
      && Rounding == 1
#endif
      ) {
    if (!word1(&u) && !(word0(&u) & Bndry_mask)
#ifndef Sudden_Underflow
        && word0(&u) & (Exp_mask & ~Exp_msk1)
#endif
        ) {
      /* The special case */
      b2 += Log2P;
      s2 += Log2P;
      spec_case = 1;
    }
  }

  /* Arrange for convenient computation of quotients:
   * shift left if necessary so divisor has 4 leading 0 bits.
   *
   * Perhaps we should just compute leading 28 bits of S once
   * and for all and pass them and a shift to quorem, so it
   * can do shifts and ors to compute the numerator for q.
   */
  i = dshift(S, s2);
  b2 += i;
  m2 += i;
  s2 += i;
  if (b2 > 0)
    b = lshift(&heap, b, b2);
  if (s2 > 0)
    S = lshift(&heap, S, s2);
  if (k_check) {
    if (cmp(b,S) < 0) {
      k--;
      b = multadd(&heap, b, 10, 0);  /* we botched the k estimate */
      if (leftright)
        mhi = multadd(&heap, mhi, 10, 0);
      ilim = ilim1;
    }
  }
  if (ilim <= 0 && (mode == 3 || mode == 5)) {
    if (ilim < 0 || cmp(b,S = multadd(&heap,S,5,0)) <= 0) {
      /* no digits, fcvt style */
    no_digits:
      k = -1 - ndigits;
      goto ret;
    }
  one_digit:
    *s++ = '1';
    k++;
    goto ret;
  }
  if (leftright) {
    if (m2 > 0)
      mhi = lshift(&heap, mhi, m2);

    /* Compute mlo -- check for special case
     * that d is a normalized power of 2.
     */

    mlo = mhi;
    if (spec_case) {
      mhi = Balloc(&heap, mhi->k);
      BCOPY(mhi, mlo);
      mhi = lshift(&heap, mhi, Log2P);
    }

    for (i = 1;  ;  i++) {
      dig = quorem(b,S) + '0';
      /* Do we yet have the shortest decimal string
       * that will round to d?
       */
      j = cmp(b, mlo);
      delta = diff(&heap, S, mhi);
      j1 = delta->sign ? 1 : cmp(b, delta);
      Bfree(&heap, delta);
#ifndef ROUND_BIASED
      if (j1 == 0 && mode != 1 && !(word1(&u) & 1)
#ifdef Honor_FLT_ROUNDS
          && Rounding >= 1
#endif
          ) {
        if (dig == '9')
          goto round_9_up;
        if (j > 0)
          dig++;
#ifdef SET_INEXACT
        else if (!b->x[0] && b->wds <= 1)
          inexact = 0;
#endif
        *s++ = dig;
        goto ret;
      }
#endif
      if (j < 0 || (j == 0 && mode != 1
#ifndef ROUND_BIASED
                    && !(word1(&u) & 1)
#endif
                    )) {
        if (!b->x[0] && b->wds <= 1) {
#ifdef SET_INEXACT
          inexact = 0;
#endif
          goto accept_dig;
        }
#ifdef Honor_FLT_ROUNDS
        if (mode > 1)
          switch(Rounding) {
          case 0: goto accept_dig;
          case 2: goto keep_dig;
          }
#endif /*Honor_FLT_ROUNDS*/
        if (j1 > 0) {
          b = lshift(&heap, b, 1);
          j1 = cmp(b, S);
#ifdef ROUND_BIASED
          if (j1 >= 0 /*)*/
#else
          if ((j1 > 0 || (j1 == 0 && dig & 1))
#endif
              && dig++ == '9')
            goto round_9_up;
        }
        accept_dig:
        *s++ = dig;
        goto ret;
      }
      if (j1 > 0) {
#ifdef Honor_FLT_ROUNDS
        if (!Rounding)
          goto accept_dig;
#endif
        if (dig == '9') { /* possible if i == 1 */
        round_9_up:
          *s++ = '9';
          goto roundoff;
        }
        *s++ = dig + 1;
        goto ret;
      }
#ifdef Honor_FLT_ROUNDS
      keep_dig:
#endif
      *s++ = dig;
      if (i == ilim)
        break;
      b = multadd(&heap, b, 10, 0);
      if (mlo == mhi)
        mlo = mhi = multadd(&heap, mhi, 10, 0);
      else {
        mlo = multadd(&heap, mlo, 10, 0);
        mhi = multadd(&heap, mhi, 10, 0);
      }
    }
  }
  else
    for(i = 1;; i++) {
      *s++ = dig = quorem(b,S) + '0';
      if (!b->x[0] && b->wds <= 1) {
#ifdef SET_INEXACT
        inexact = 0;
#endif
        goto ret;
      }
      if (i >= ilim)
        break;
      b = multadd(&heap, b, 10, 0);
    }

  /* Round off last digit */

#ifdef Honor_FLT_ROUNDS
  switch(Rounding) {
   case 0: goto trimzeros;
   case 2: goto roundoff;
  }
#endif
  b = lshift(&heap, b, 1);
  j = cmp(b, S);
#ifdef ROUND_BIASED
  if (j >= 0)
#else
  if (j > 0 || (j == 0 && dig & 1))
#endif
    {
    roundoff:
      while(*--s == '9')
        if (s == s0) {
          k++;
          *s++ = '1';
          goto ret;
        }
      ++*s++;
    }
  else {
#ifdef Honor_FLT_ROUNDS
  trimzeros:
#endif
    while(*--s == '0');
    s++;
  }
 ret:
  Bfree(&heap, S);
  if (mhi) {
    if (mlo && mlo != mhi)
      Bfree(&heap, mlo);
    Bfree(&heap, mhi);
  }
 ret1:
#ifdef SET_INEXACT
  if (inexact) {
    if (!oldinexact) {
      word0(&u) = Exp_1 + (70 << Exp_shift);
      word1(&u) = 0;
      dval(&u) += 1.;
    }
  }
  else if (!oldinexact)
    clear_inexact();
#endif
  Bfree(&heap, b);
  *s = 0;
  *decpt = k + 1;
  if (rve)
    *rve = s;
  return s0;
} /* dtoaLoc */

/* dump some info about the flags used to build dtoa, etc.
 * return 0 for OK, ~0 if there are issues detected
 */
int DTOA_API DTOA_CALLCONV
dtoaInfoLoc ()
{
  int rc = 0;
  U w, x;
  double r, s, t;
  int rc2;

  printf ("\n");
  printf ("------------------ dtoaInfoLoc ------------------\n");
  printf (" DTOA_USE_ND_BOUND : %d (%s)\n", (int) DTOA_USE_ND_BOUND,
          (1==DTOA_USE_ND_BOUND) ? "yes" :
          (0==DTOA_USE_ND_BOUND) ? "no" : "???");
#if 1 == DTOA_USE_ND_BOUND
  printf ("     DTOA_ND_BOUND : %d\n", (int) DTOA_ND_BOUND);
#endif
  printf ("  Avoid_Underflow  : only mode included now\n");
#if defined(Sudden_Underflow)
  printf ("  Sudden_Underflow :     defined\n");
#else
  printf ("  Sudden_Underflow : NOT defined\n");
#endif
  printf ("           DBL_DIG :     %d\n", DBL_DIG);

#if defined(Honor_FLT_ROUNDS)
  printf ("  Honor_FLT_ROUNDS :     defined\n");
#else
  printf ("  Honor_FLT_ROUNDS : NOT defined\n");
#endif
#if defined(Trust_FLT_ROUNDS)
  printf ("  Trust_FLT_ROUNDS :     defined\n");
#else
  printf ("  Trust_FLT_ROUNDS : NOT defined\n");
#endif
#if defined(Check_FLT_ROUNDS)
  printf ("  Check_FLT_ROUNDS :     defined\n");
#else
  printf ("  Check_FLT_ROUNDS : NOT defined\n");
#endif
  printf ("Honor_FLT_ROUNDS toggles whether we use the current rounding mode\n");
  printf ("[Trust|Check]_FLT_ROUNDS controls how we get the current rounding mode\n");
  printf ("\n");

  rc2 = 1;                      /* start underflow test */
#ifdef IEEE_8087
  w.L[1] = 0x00300000;
  w.L[0] = 0;
  x.L[1] = 0x1ff00000;
  x.L[0] = 0;
#else
  w.L[0] = 0x00300000;
  w.L[1] = 0;
  x.L[0] = 0x1ff00000;
  x.L[1] = 0;
#endif
  /* set x = 2^(-512) ~= 7.549e-155,
   * w = (4x)^2 = 2^(-1020)
   */
#if 0
  printf (" %%g  : %26.19g\n", x.d);
  printf (" x32 : %08x %08x\n", x.L[1], x.L[0]);
  {
    U tt;
    tt.d = 1e-155;
    printf (" %%g  : %26.19g\n", tt.d);
    printf (" x32 : %08x %08x\n", tt.L[1], tt.L[0]);
  }
#endif
  t = x.d;
  /* to compute r, square first, then bump up: will rely on denormals */
  r = t * t;
  r *= 4;
  r *= 4;

  /* to compute s, first bump up, then square: stay normalized */
  t *= 4;
  s = t * t;
  /* printf ("r = %g  s = %g\n", r, s); */
  /* y.d = s; */
  /* printf (" %%g  : %26.19g\n", y.d); */
  /* printf (" x32 : %08x %08x\n", y.L[1], y.L[0]); */
  if ((0 == s)) {
    printf ("Checking underflow behavior: Test is broken!!\n");
  }
  else if (w.d != s) {
    printf ("Checking underflow behavior: Test is still broken!!\n");
  }
  else {
    if (0 == r) {
      printf ("Checking underflow behavior: SUDDEN UNDERFLOW detected\n");
#if defined(Sudden_Underflow)
      rc2 = 0;
#else
      printf ("                 **** ERROR: Sudden_Underflow not defined\n");
#endif
    }
    else if (r == w.d) {
      printf ("Checking underflow behavior: GRADUAL UNDERFLOW detected\n");
#if defined(Sudden_Underflow)
      printf ("                 **** ERROR: Sudden_Underflow is defined\n");
      /* IMHO the code included with -DSudden_Underflow fails
       * if the underflow is gradual */
#else
      rc2 = 0;
#endif
    }
    else
      printf ("Checking underflow behavior: What kind of underflow is this?!?!?\n");
  }
  if (rc2)
    rc = 1;

  printf ("             HEAP_SZ : %d doubles\n", HEAP_SZ);
#if defined(DTOA_INFO_LOC)
  printf (" heap highwater mark : %u doubles\n", heapHW);
  printf ("    k highwater mark : %d\n", kHW);
#endif  /* defined(DTOA_INFO_LOC) */
  printf ("----------------------------------------------\n");

  return rc;
} /* dtoaInfoLoc */
