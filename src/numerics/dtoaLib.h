#if ! defined(_DTOA_LIB_H_)
#     define  _DTOA_LIB_H_

#if   defined(_WIN32)
# if defined(DTOA_EXPORTS)
#  define DTOA_API __declspec(dllexport)
# else
#  define DTOA_API __declspec(dllexport)
# endif
# define DTOA_CALLCONV __stdcall
#elif defined(__GNUC__)
# if defined(DTOA_EXPORTS)
#  define DTOA_API __attribute__((__visibility__("default")))
# else
#  define DTOA_API
# endif
# define DTOA_CALLCONV
#else
# define DTOA_API
# define DTOA_CALLCONV
#endif

#if defined(__cplusplus)
extern "C" {
#endif

  double DTOA_API DTOA_CALLCONV
  strtodLoc (const char *s00, char **se, int *locErrno);

  double DTOA_API DTOA_CALLCONV
  strtodSS (const char *ss, int *nUsed, int *rangeError);

  char DTOA_API * DTOA_CALLCONV
  dtoaLoc (double dd, int mode, int ndigits,
           char buf[], size_t bufSiz, int *decpt, int *sign, char **rve);

  void DTOA_API DTOA_CALLCONV
  dtoaLocPas (double dd, char *ss, int *digCount, int *decPos, int *isNeg);

  int DTOA_API DTOA_CALLCONV
  dtoaInfoLoc ();

#if defined(__cplusplus)
}
#endif

#endif /* defined(_DTOA_LIB_H_) */
