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
