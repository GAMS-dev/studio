/* doubleFormat.h
 * Feb 2020: Formatting routines for displaying doubles
 */

char *x2fixed (double v, int nDecimals, int squeeze, char outBuf[], int *outLen);

char *x2efmt (double v, int nSigFigs, int squeeze, char outBuf[], int *outLen);

char *x2gfmt (double v, int nSigFigs, int squeeze, char outBuf[], int *outLen);
