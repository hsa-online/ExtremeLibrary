/* Extreme Library (EL). Dynamic strings. 
 * Copyright (c) 2014 Sergei Hrushev [hrushev DOG gmail DOT com]
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _EL_STR_H_
#define _EL_STR_H_

#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** 
 * @brief Holds the data required to represent single dynamic string.
 *
 * If dynamic string is created using any elstrCreateXX routine except the 
 * elstrCreateFixed() it becomes "variable size" dynamic string. This means that 
 * the buffer size is allocated "when necessary" and may grow.
 *
 * Fixed strings use buffers preallocated externally and are unable to grow. 
 * These strings are extremely helpful when data locality is important so they
 * may work much faster in some circumstances. 
 */
typedef struct str {
	size_t nLength; /**< Length of the string (in bytes). */
	size_t nCapacity; /**< Amount of memory allocated for the data buffer 
	(in bytes). */
	size_t nExtra; /**< Two low order bits are now used for flags. All the rest 
	is reserved to support strings in multibyte encoding. */
 	char *szBuf; /**< The data buffer itself. */
} str;

void elstrArrayELStrDestroy(str **pStrings, size_t nCountStrings);
size_t elstrMBGetMaxLength();

str *elstrCreateEmpty();
str *elstrCreateEmptyWithCapacity(size_t nCapacity);
str *elstrCreateFromCStr(const char *sz);
str *elstrCreateFromELStr(str *pStr);
str *elstrCreateFromCSubStr(const char *sz, int nIndex, size_t nCount);
str *elstrCreateFromELSubStr(str *pStr, int nIndex, size_t nCount);
str *elstrCreateFromFileCStr(const char *szFullName);
str *elstrCreateFromFileELStr(str *pStr);
str *elstrCreateFixed(char *szBufferToUse, size_t nCapacity);
void elstrDestroy(str *pThis);
void elstrEnsureCapacity(str *pThis, size_t nCapacity);
void elstrRemoveExtraCapacity(str *pThis);
bool elstrIsEmpty(str *pThis);
size_t elstrGetLength(str *pThis);
size_t elstrMBGetLength(str *pThis);
void elstrSetLength(str *pThis, size_t nLength);
void elstrClear(str *pThis);
size_t elstrGetCapacity(str *pThis);
size_t elstrGetUnused(str *pThis);
const char *elstrGetRawBuf(str *pThis);
str *elstrSubString(str *pThis, int nIndex, size_t nCount);
void elstrAssignFromCStr(str *pThis, const char *sz);
void elstrAssignFromELStr(str *pThis, str *pStr);
void elstrAppendCStr(str *pThis, char *sz);
void elstrAppendELStr(str *pThis, str *pStr);
void elstrPrependCStr(str *pThis, char *sz);
void elstrPrependELStr(str *pThis, str *pStr);
void elstrAppendAllCStr(str *pThis, ...);
void elstrAppendAllELStrPrealloc(str *pThis, ...);
void elstrAppendPrintfCStrFormat(str *pThis, const char *cszFormat, ...);
void elstrAppendPrintfELStrFormat(str *pThis, str *pStrFormat, ...);
void elstrInsertCStr(str *pThis, int nIndex, char *sz);
void elstrDelete(str *pThis, int nIndex, size_t nCount);
void elstrLTrimChars(str *pThis, char arrChars[], size_t nCountChars);
void elstrLTrim(str *pThis);
void elstrRTrimChars(str *pThis, char arrChars[], size_t nCountChars);
void elstrRTrim(str *pThis);
void elstrTrim(str *pThis);
void elstrReverse(str *pThis);
int elstrCompareCStr(str *pThis, const char *sz);
bool elstrHasPrefixCStr(str *pThis, char *sz);
bool elstrHasPrefixELStr(str *pThis, str *pStr);
bool elstrHasSuffixCStr(str *pThis, char *sz);
bool elstrHasSuffixELStr(str *pThis, str *pStr);
int elstrIndexOfChar(str *pThis, char ch);
int elstrIndexOfCharFrom(str *pThis, char ch, int nIndexFrom);
str **elstrSplitByChars(str *pThis, char arrChars[], size_t nCountChars, 
	bool bRemoveEmpty, size_t *pCountSubstrings);
str **elstrSplitByCharsNoEmpty(str *pThis, char arrChars[], size_t nCountChars, 
	size_t *pCountSubstrings);

#ifdef __cplusplus
}
#endif

#endif
