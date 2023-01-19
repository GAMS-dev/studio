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
#if ! defined(_DTOA_LOC_H_)
#     define  _DTOA_LOC_H_

#define DTOA_API
#define DTOA_CALLCONV

#if defined(__cplusplus)
extern "C" {
#endif

  double DTOA_API DTOA_CALLCONV
  strtodLoc (const char *s00, char **se, int *locErrno);

  char DTOA_API * DTOA_CALLCONV
  dtoaLoc (double dd, int mode, int ndigits,
           char buf[], size_t bufSiz, int *decpt, int *sign, char **rve);

  int DTOA_API DTOA_CALLCONV
  dtoaInfoLoc ();

#if defined(__cplusplus)
}
#endif

#endif /* defined(_DTOA_LOC_H_) */
