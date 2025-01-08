/**
 * GAMS Studio
 *
 * Copyright (c) 2017-2025 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2025 GAMS Development Corp. <support@gams.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/* doubleFormat.h
 * Feb 2020: Formatting routines for displaying doubles
 */

#ifdef __cplusplus
extern "C" {
#endif

char *x2fixed (double v, int nDecimals, int squeeze, char outBuf[], int *outLen, char decSep);

char *x2efmt (double v, int nSigFigs, int squeeze, char outBuf[], int *outLen, char decSep);

char *x2gfmt (double v, int nSigFigs, int squeeze, char outBuf[], int *outLen, char decSep);

#ifdef __cplusplus
};
#endif
