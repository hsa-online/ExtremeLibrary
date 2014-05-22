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

#include <stdlib.h>
#include <string.h>
#include <stdio.h> 
#include <stdarg.h>
#include <wchar.h>
#include <stdint.h>

#include "el_str.h"

/**
 * Number of bits in @e str.nExtra used by flags.
 */
#define EL_STR_NUM_FLAGS		3
/**
 * "Not A String" flag.
 */
#define EL_STR_FLAG_NAS			1
/**
 * "String uses a fixed buffer" flag (string data buffer is externally 
 * allocated).
 */
#define EL_STR_FLAG_FIXED		2
/**
 * "String structure is preallocated" flag ("str" structure is externally 
 * allocated).
 */
#define EL_STR_FLAG_PREALLOC	4
 
#define EL_STR_MB_LENGTH_MAX (SIZE_MAX >> EL_STR_NUM_FLAGS)

#define isNaS(s) (((s)->nExtra & EL_STR_FLAG_NAS) == EL_STR_FLAG_NAS)
#define isFixed(s) (((s)->nExtra & EL_STR_FLAG_FIXED) == EL_STR_FLAG_FIXED)
#define isPreallocated(s) (((s)->nExtra & EL_STR_FLAG_PREALLOC) == \
 	EL_STR_FLAG_PREALLOC)
#define makeNaS(s) { \
    if(!isFixed(s) && (s)->szBuf != NULL) \
    	{ free((s)->szBuf); (s)->szBuf = NULL; }\
 	(s)->nExtra |= EL_STR_FLAG_NAS; }
#define setMBLength(s, nLength) (s)->nExtra |= ((nLength) << EL_STR_NUM_FLAGS)
#define getMBLength(s) ((s)->nExtra >> EL_STR_NUM_FLAGS)
#define clearMBLength(s) (s)->nExtra &= \
 	~(EL_STR_MB_LENGTH_MAX << EL_STR_NUM_FLAGS)

/**
 * Creates new empty string with minimal possible capacity.
 * @return Newly created dynamic string (or NULL if error occured).
 */
str *elstrCreateEmpty() {
	return elstrCreateEmptyWithCapacity(1);
}

/**
 * Creates new empty string with a specified capacity.
 * Capacity should be 1 or more.
 * @param  nCapacity String initial capacity in bytes.
 * @return           Newly created dynamic string (or NULL if an error occured).
 */
str *elstrCreateEmptyWithCapacity(size_t nCapacity) {
	if(nCapacity == 0)
		return NULL;

	str *pThis = calloc(1, sizeof(str));
	if(pThis == NULL)
		return NULL;

	elstrEnsureCapacity(pThis, nCapacity);
	if(isNaS(pThis)) {
		free(pThis);
		return NULL;
	}

	elstrSetLength(pThis, 0);

	return pThis;
}

/**
 * Creates new string and initializes it with the value of the specified 
 * C string.
 * @param  sz The C string to copy data from.
 * @return    Newly created dynamic string (or NULL if an error occured).
 */
str *elstrCreateFromCStr(const char *sz) {
	if(sz == NULL)
		return NULL;

	size_t nLen = strlen(sz);

	str *pThis = calloc(1, sizeof(str));
	if(pThis == NULL)
		return NULL;

	elstrEnsureCapacity(pThis, nLen + 1);
	if(isNaS(pThis)) {
		free(pThis);
		return NULL;
	}

	strncpy(pThis->szBuf, sz, nLen);

	elstrSetLength(pThis, nLen);

	return pThis;
}

/**
 * Creates new string and initializes it with a value of the specified 
 * dynamic string.
 * @param  pStr The dynamic string to copy data from.
 * @return      Newly created dynamic string (or NULL if an error occured).
 */
str *elstrCreateFromELStr(str *pStr) {
	if(pStr == NULL || isNaS(pStr))
		return NULL;

	str *pThis = calloc(1, sizeof(str));
	if(pThis == NULL)
		return NULL;

	elstrEnsureCapacity(pThis, pStr->nLength + 1);
	if(isNaS(pThis)) {
		free(pThis);
		return NULL;
	}

	strncpy(pThis->szBuf, pStr->szBuf, pStr->nLength);

	elstrSetLength(pThis, pStr->nLength);

	return pThis;
}

/**
 * Creates new string and initializes it with substring of the specified 
 * C string.
 * @param  sz     The C style string to copy data from.
 * @param  nIndex An index of the fisrt byte to be copied.
 * @param  nCount Number of bytes to copy.
 * @return        Newly created dynamic string (or NULL if an error occured).
 */
str *elstrCreateFromCSubStr(const char *sz, int nIndex, size_t nCount) {
	if(sz == NULL)
		return NULL;
	size_t nLen = strlen(sz);

	if(nIndex < 0 || nIndex > nLen)
		return NULL;

	if(nIndex == nLen || nCount == 0)
		return elstrCreateEmpty();

	if(nIndex + nCount > nLen)
		nCount = nLen - nIndex;

	str *pThis = calloc(1, sizeof(str));
	if(pThis == NULL)
		return NULL;

	elstrEnsureCapacity(pThis, nCount + 1);
	if(isNaS(pThis)) {
		free(pThis);
		return NULL;
	}

	strncpy(pThis->szBuf, sz + nIndex, nCount);

	elstrSetLength(pThis, nCount);

	return pThis;
}

/**
 * Creates new string and initializes it with substring of the specified 
 * dynamic string.
 * @param  pStr   The dynamic string to copy data from.
 * @param  nIndex An index of the fisrt byte to be copied.
 * @param  nCount Number of bytes to copy.
 * @return        Newly created dynamic string (or NULL if an error occured).
 */
str *elstrCreateFromELSubStr(str *pStr, int nIndex, size_t nCount) {
	if(pStr == NULL || isNaS(pStr))
		return NULL;

	if(nIndex < 0 || nIndex > pStr->nLength)
		return NULL;

	if(nIndex == pStr->nLength || nCount == 0)
		return elstrCreateEmpty();

	if(nIndex + nCount > pStr->nLength)
		nCount = pStr->nLength - nIndex;

	str *pThis = calloc(1, sizeof(str));
	if(pThis == NULL)
		return NULL;

	elstrEnsureCapacity(pThis, nCount + 1);
	if(isNaS(pThis)) {
		free(pThis);
		return NULL;
	}

	strncpy(pThis->szBuf, pStr->szBuf + nIndex, nCount);

	elstrSetLength(pThis, nCount);

	return pThis;
}

/**
 * Creates new string and initializes it with contents of the file. File name is
 * specified as a C string.
 * @param  szFullName File name to load the string data from.
 * @return            Newly created dynamic string (or NULL if an error 
 * occured).
 */
str *elstrCreateFromFileCStr(const char *szFullName) {
	if(szFullName == NULL)
		return NULL;
	size_t nLen = strlen(szFullName);

	if(nLen == 0)
		return NULL;

	FILE *stream = fopen(szFullName, "r");
	if(stream == NULL)
		return NULL;

	if(fseek(stream, 0L, SEEK_END) != 0) {
		fclose(stream);
		return NULL;
	}

	long nnSize = ftell(stream);
	if(nnSize == -1) {
		fclose(stream);
		return NULL;
	}

	if(fseek(stream, 0L, SEEK_SET) != 0) {
		fclose(stream);
		return NULL;
	}

	str *pThis = calloc(1, sizeof(str));
	if(pThis == NULL) {
		fclose(stream);
		return NULL;
	}

	elstrEnsureCapacity(pThis, nnSize + 1);
	if(isNaS(pThis)) {
		fclose(stream);
		free(pThis);
		return NULL;
	}

	size_t nCount = fread(pThis->szBuf, 1, nnSize, stream);
	if(nCount < nnSize) {
		fclose(stream);
		makeNaS(pThis);
		free(pThis);
		return NULL;
	}

	fclose(stream);

	elstrSetLength(pThis, nnSize);

	return pThis;
}

/**
 * Creates new string and initializes it with contents of the file. File name is
 * specified as dynamic string.
 * @param  pStr File name to load the string data from.
 * @return      Newly created dynamic string (or NULL if an error occured).
 */
str *elstrCreateFromFileELStr(str *pStr) {
	if(pStr == NULL || isNaS(pStr))
		return NULL;

	return elstrCreateFromFileCStr(elstrGetRawBuf(pStr));
}

/**
 * Creates new empty string with a @b fixed capacity equal to @e nCapacity. 
 * Because the string is "fixed" it will never grow above the @e nCapacity. 
 * String will use the externally allocated memory buffer which must be 
 * provided.
 * @param  szBufferToUse Pointer to memory buffer where string will hold its 
 * data. Buffer must exist all the string lifetime. Size of the buffer must be 
 * not less than @e nCapacity of bytes.
 * @param  nCapacity     String fixed capacity in bytes. <br> Note that 
 * specifying @e nCapacity=1 creates string which won't be able to hold any 
 * data. 
 * @return               Newly created dynamic string (or NULL if an error 
 * occured).
 */
str *elstrCreateEmptyFixed(char *szBufferToUse, size_t nCapacity) {
	if(szBufferToUse == NULL || nCapacity == 0) 
		return NULL;

	str *pThis = calloc(1, sizeof(str));
	if(pThis == NULL)
		return NULL;

	pThis->szBuf = szBufferToUse;
	pThis->nCapacity = nCapacity;
	pThis->nExtra |= EL_STR_FLAG_FIXED;

	elstrSetLength(pThis, 0);

	return pThis;
}

/**
 * Creates new empty preallocated dynamic string. Uses externally allocated 
 * buffer to hold the "str" structure. String capacity is @b fixed and is equal 
 * to @e nCapacity. Because the string is "fixed" it will never grow above 
 * the @e nCapacity. String will use the externally allocated memory buffer to
 * hold its data.
 * @param  p             Pointer to memory buffer where the "str" structure will
 * be placed. 
 * @param  szBufferToUse Pointer to memory buffer where string will hold its 
 * data. Buffer must exist all the string lifetime. Size of the buffer must be 
 * not less than @e nCapacity of bytes.
 * @param  nCapacity     String fixed capacity in bytes. <br> Note that 
 * specifying @e nCapacity=1 creates string which won't be able to hold any 
 * data. 
 * @return               Newly created dynamic string (or NULL if an error 
 * occured).
 */
str *elstrCreateEmptyPreallocFixed(void *p, char *szBufferToUse, 
	size_t nCapacity) {

	if(p == NULL || szBufferToUse == NULL || nCapacity == 0) 
		return NULL;

	str *pThis = p;

	pThis->szBuf = szBufferToUse;
	pThis->nCapacity = nCapacity;
	pThis->nExtra = EL_STR_FLAG_PREALLOC | EL_STR_FLAG_FIXED;

	elstrSetLength(pThis, 0);

	return pThis;
}

/**
 * Creates new empty preallocated dynamic string. Uses externally allocated 
 * buffer to hold the "str" structure.
 * @param  p Pointer to memory buffer where the "str" structure will be placed. 
 * Size of this buffer should be equal to sizeof(str).
 * @return   Newly created dynamic string (or NULL if an error occured).
 */
str *elstrCreateEmptyPrealloc(void *p) {
	return elstrCreateEmptyPreallocWithCapacity(p, 1);
}
/**
 * Creates new empty preallocated dynamic string with a specified capacity. Uses 
 * externally allocated buffer to hold the "str" structure.
 * @param  p         Pointer to memory buffer where the "str" structure will be 
 * placed.
 * @param  nCapacity String capacity in bytes.
 * @return           Newly created dynamic string (or NULL if an error occured).
 */
str *elstrCreateEmptyPreallocWithCapacity(void *p, size_t nCapacity) {
	if(p == NULL || nCapacity == 0) 
		return NULL;

	str *pThis = p;

	elstrEnsureCapacity(pThis, nCapacity);
	if(isNaS(pThis)) {
		return NULL;
	}
	pThis->nExtra |= EL_STR_FLAG_PREALLOC;

	elstrSetLength(pThis, 0);

	return pThis;
}

/**
 * Destroys the dynamic string.
 * @param pThis Dynamic string to be destroyed.
 */
void elstrDestroy(str *pThis) {
	if(pThis == NULL)
		return;

	if(!isFixed(pThis) && pThis->szBuf != NULL) 
		free(pThis->szBuf);
	if(!isPreallocated(pThis)) 
		free(pThis);	
}

/**
 * Changes capacity of the string buffer to ensure that it can hold at least 
 * nCapacity of bytes (including '\\0'). If current capacity is enough - does 
 * nothing.
 * @param pThis     Dynamic string.
 * @param nCapacity Required capacity.
 */
void elstrEnsureCapacity(str *pThis, size_t nCapacity) {
	if(isNaS(pThis))
		return;

	if(pThis->nCapacity < nCapacity) {
		if(isFixed(pThis)) {
			makeNaS(pThis);
		} else {
			char *szBufNew = realloc(pThis->szBuf, nCapacity);
			if(szBufNew == NULL) {
				makeNaS(pThis);
			} else {
				pThis->szBuf = szBufNew;
				pThis->nCapacity = nCapacity;
			}
		}
	}
}

/**
 * Truncates the data buffer to actual length of string. Does nothing on empty 
 * strings. Also does nothing on fixed string as their capacity can't be 
 * changed.
 * @param pThis Dynamic string.
 */
void elstrRemoveExtraCapacity(str *pThis) {
	if(isNaS(pThis))
		return;

	if(isFixed(pThis))
		return;

	size_t nCapacity = pThis->nLength + 1;
	if(pThis->nCapacity > nCapacity) {
		char *szBufNew = realloc(pThis->szBuf, nCapacity);
		if(szBufNew == NULL) {
			makeNaS(pThis);
		} else {
			pThis->szBuf = szBufNew;
			pThis->nCapacity = nCapacity;
		}
	}
}

/** 
 * Returns true if the string is empty.
 * @param  pThis Dynamic string.
 * @return       True if the string is empty.
 */
bool elstrIsEmpty(str *pThis) {
	if(isNaS(pThis))
		return true;
	return pThis->nLength == 0;
}

/**
 * Returns the length of string.
 * @param  pThis Dynamic string.
 * @return       Length of the string in bytes.
 */
size_t elstrGetLength(str *pThis) {
	if(isNaS(pThis))
		return 0;
	return pThis->nLength;
}

/**
 * Returns the length of dynamic string containing multibyte characters.
 * @param  pThis Dynamic string.
 * @return       Length of the string in characters.
 */
size_t elstrMBGetLength(str *pThis) {
	if(isNaS(pThis))
		return 0;
	if(pThis->nLength == 0)
		return 0;

	size_t nLength = getMBLength(pThis);
	if(nLength != 0)
		return nLength;

	mbstate_t mbs;
	// mbrlen(NULL, 0, &mbs); - doesn't work in CentOS 6.5
	memset(&mbs, 0, sizeof(mbstate_t));

	char *szBuf = pThis->szBuf;
	size_t nMax = pThis->nLength;
	while(nMax > 0) {
		size_t nLengthCur = mbrlen(szBuf, nMax, &mbs);
		if(nLengthCur == (size_t)(0) || nLengthCur == (size_t)(-1) || 
			nLengthCur == (size_t)(-2)) {

			makeNaS(pThis);
			return 0;
		}
		szBuf += nLengthCur;
		nMax -= nLengthCur;
		nLength++;
	}

	if(nLength > EL_STR_MB_LENGTH_MAX) {
		makeNaS(pThis);
		return 0;
	}

	setMBLength(pThis, nLength);

	return nLength;
}

/** 
 * Take with care!!!<br>
 * Ensures the string has enough space to hold at least @e nLength bytes.
 * Enlarges string if necessary. Sets the length of string to the value 
 * specified. If string is enlarged all added content is undefined. It's 
 * guaranteed  that the string is always null-terminated.
 * @param pThis   Dynamic string.
 * @param nLength Desired length of the string in bytes.
 */
void elstrSetLength(str *pThis, size_t nLength) {
	if(isNaS(pThis))
		return;

	elstrEnsureCapacity(pThis, nLength + 1);
	if(isNaS(pThis))
		return;

	pThis->nLength = nLength;
	pThis->szBuf[pThis->nLength] = '\0';

	clearMBLength(pThis);
}

// 
/** 
 * Sets the length of the string to 0 (this makes the string "empty").
 * <br>Not truncates the data buffer!
 * @param pThis Dynamic string.
 */
void elstrClear(str *pThis) {
	if(isNaS(pThis))
		return;

	elstrSetLength(pThis, 0);
}

/**
 * Returns the capacity of string in bytes.
 * @param  pThis Dynamic string.
 * @return       String capacity in bytes.
 */
size_t elstrGetCapacity(str *pThis) {
	if(isNaS(pThis))
		return 0;
	return pThis->nCapacity;
}

/**
 * Returns an amount of memory (in bytes) allocated for the dynamic string but 
 * not used now.
 * @param  pThis Dynamic string.
 * @return       An amount of allocated but unused memory (in bytes).
 */
size_t elstrGetUnused(str *pThis) {
	if(isNaS(pThis))
		return 0;
	return elstrGetCapacity(pThis) - elstrGetLength(pThis) - 1;
}

/**
 * Take with care!!!<br>
 * Returns constant pointer to the data buffer of dynamic string. Do not write more than 
 * elstrGetCapacity() - 1 bytes to the buffer (-1 is to reserve place for 
 * '\\0'). Do not modify any data in the buffer returned.
 * @param  pThis Dynamic string.
 * @return       Pointer to the data buffer of dynamic string.
 */
const char *elstrGetRawBuf(str *pThis) {
	if(isNaS(pThis))
		return NULL;
	return pThis->szBuf;
}

/**
 * Creates new string from the substring of dynamic string.
 * @note 
 * Even when the @e pThis dynamic string is "fixed". This function still creates
 * its substring as a "variable size" dynamic string.
 * @param  pThis  Dynamic string.
 * @param  nIndex An index of the first character to copy to substring.
 * @param  nCount Number of characters in substring.
 * @return        New dynamic string or NULL if an error occured.
 */
str *elstrSubString(str *pThis, int nIndex, size_t nCount) {
	return elstrCreateFromELSubStr(pThis, nIndex, nCount);
}

/**
 * Assigns specified C string (@e sz) to the dynamic string.
 * @param pThis Dynamic string.
 * @param sz    C string to get the data from.
 */
void elstrAssignFromCStr(str *pThis, const char *sz) {
	if(isNaS(pThis))
		return;

	if(sz == NULL)
		return;
	size_t nLen = strlen(sz);

	elstrEnsureCapacity(pThis, nLen + 1);
	if(isNaS(pThis))
		return;

	strncpy(pThis->szBuf, sz, nLen);

	elstrSetLength(pThis, nLen);
}

/**
 * Assigns specified Dynamic string (@e pStr) to the dynamic string.
 * @param pThis Dynamic string.
 * @param pStr  Dynamic string to get the data from.
 */
void elstrAssignFromELStr(str *pThis, str *pStr) {
	if(isNaS(pThis))
		return;

	if(pStr == NULL || isNaS(pStr))
		return;

	elstrEnsureCapacity(pThis, pStr->nLength + 1);
	if(isNaS(pThis))
		return;

	strncpy(pThis->szBuf, pStr->szBuf, pStr->nLength);

	elstrSetLength(pThis, pStr->nLength);
}

/**
 * Appends the specified C string (@e sz) to dynamic string.
 * @param pThis Dynamic string.
 * @param sz    C string to append.
 */
void elstrAppendCStr(str *pThis, char *sz) {
	if(isNaS(pThis))
		return;

	if(sz == NULL)
		return;
	size_t nLen = strlen(sz);

	elstrEnsureCapacity(pThis, pThis->nLength + nLen + 1);
	if(isNaS(pThis))
		return;

	strncpy(pThis->szBuf + pThis->nLength, sz, nLen);

	elstrSetLength(pThis, pThis->nLength + nLen);
}

/**
 * Appends the specified dynamic string (@e pStr) to dynamic string.
 * @param pThis Dynamic string.
 * @param pStr  Dynamic string to append.
 */
void elstrAppendELStr(str *pThis, str *pStr) {
	if(isNaS(pThis))
		return;

	if(pStr == NULL || isNaS(pStr))
		return;

	elstrEnsureCapacity(pThis, pThis->nLength + pStr->nLength + 1);
	if(isNaS(pThis))
		return;

	strncpy(pThis->szBuf + pThis->nLength, pStr->szBuf, pStr->nLength);

	elstrSetLength(pThis, pThis->nLength + pStr->nLength);
}

/**
 * Inserts the specified C string (@e sz) at the beginning of dynamic string.
 * @param pThis  Dynamic string.
 * @param sz     C string to insert.
 */
void elstrPrependCStr(str *pThis, char *sz) {
	if(isNaS(pThis))
		return;

	if(sz == NULL)
		return;
	size_t nLen = strlen(sz);

	elstrEnsureCapacity(pThis, pThis->nLength + nLen + 1);
	if(isNaS(pThis))
		return;

	memmove(pThis->szBuf + nLen, pThis->szBuf, pThis->nLength);
	strncpy(pThis->szBuf, sz, nLen);

	elstrSetLength(pThis, pThis->nLength + nLen);
}

/**
 * Inserts the specified dynamic string (@e pStr) at the beginning of dynamic 
 * string.
 * @param pThis  Dynamic string.
 * @param pStr   Dynamic string to insert.
 */
void elstrPrependELStr(str *pThis, str *pStr) {
	if(isNaS(pThis))
		return;

	if(pStr == NULL || isNaS(pStr))
		return;

	elstrEnsureCapacity(pThis, pThis->nLength + pStr->nLength + 1);
	if(isNaS(pThis))
		return;

	memmove(pThis->szBuf + pStr->nLength, pThis->szBuf, pThis->nLength);
	strncpy(pThis->szBuf, pStr->szBuf, pStr->nLength);

	elstrSetLength(pThis, pThis->nLength + pStr->nLength);
}

/**
 * Appends all specified C strings to dynamic string. The list of strings must 
 * end with NULL.
 * @param pThis  Dynamic string.
 * @param ...    C strings to append. Last string should be NULL.
 */
void elstrAppendAllCStr(str *pThis, ...) {
	if(isNaS(pThis))
		return;

	va_list vl;
	va_start(vl, pThis);
	
	char *sz;
	for(sz = va_arg(vl, char *); sz; sz = va_arg(vl, char *))
		elstrAppendCStr(pThis, sz);
	
	va_end(vl);
}

/**
 * Appends all specified dynamic strings to dynamic string. The list of strings 
 * must end with NULL. Preallocates all required amount of memory at once.
 * @param pThis  Dynamic string.
 * @param ...    Dynamic strings to append. Last string should be NULL.
 */
void elstrAppendAllELStrPrealloc(str *pThis, ...) {
	if(isNaS(pThis))
		return;

	int nCapacity = pThis->nLength + 1;

	va_list vl;
	va_start(vl, pThis);
	
	str *pStr;
	for(pStr = va_arg(vl, str *); pStr; pStr = va_arg(vl, str *))
		nCapacity += elstrGetLength(pStr);
	
	va_end(vl);

	elstrEnsureCapacity(pThis, nCapacity);
	if(isNaS(pThis))
		return;

	va_start(vl, pThis);
	
	for(pStr = va_arg(vl, str *); pStr; pStr = va_arg(vl, str *))
		elstrAppendELStr(pThis, pStr);
	
	va_end(vl);
}

/**
 * Appends formatted data to the dynamic string using a C format string. 
 * The format string (@e cszFormat) is described in vsnprintf docs. 
 * @param pThis     Dynamic string.
 * @param cszFormat Format C string.
 * @param ...       Arguments to be formatted.
 */
void elstrAppendPrintfCStrFormat(str *pThis, const char *cszFormat, ...) {
	if(isNaS(pThis))
		return;

	if(cszFormat == NULL || strlen(cszFormat) == 0) {
		makeNaS(pThis);
		return;
	}

	va_list vl;
	while (1) 
	{
		va_start(vl, cszFormat); 
		int nWritten = vsnprintf(pThis->szBuf + pThis->nLength, 
			pThis->nCapacity - pThis->nLength, cszFormat, vl);
		va_end(vl);
		if (nWritten > -1 && nWritten < pThis->nCapacity - pThis->nLength) {
			elstrSetLength(pThis, pThis->nLength + nWritten);
			return;
		}
		else
			{
				elstrEnsureCapacity(pThis, pThis->nCapacity * 2);
				if(isNaS(pThis))
					return;
			}
	}
}

/**
 * Appends formatted data to the dynamic string using a dynamic format string. 
 * The format string (@e cszFormat) is described in vsnprintf docs. 
 * @param pThis      Dynamic string.
 * @param pStrFormat Format dynamic string.
 * @param ...        Arguments.
 */
void elstrAppendPrintfELStrFormat(str *pThis, str *pStrFormat, ...) {
	if(isNaS(pThis))
		return;

	if(pStrFormat == NULL || isNaS(pStrFormat)) {
		makeNaS(pThis);
		return;
	}

	va_list vl;
	while (1) 
	{
		va_start(vl, pStrFormat); 
		int nWritten = vsnprintf(pThis->szBuf + pThis->nLength, 
			pThis->nCapacity - pThis->nLength, elstrGetRawBuf(pStrFormat), vl);
		va_end(vl);
		if (nWritten > -1 && nWritten < pThis->nCapacity - pThis->nLength) {
			elstrSetLength(pThis, pThis->nLength + nWritten);
			return;
		}
		else
			{
				elstrEnsureCapacity(pThis, pThis->nCapacity * 2);
				if(isNaS(pThis))
					return;
			}
	}
}

/**
 * Inserts the specified C string at the specified index of this string.
 * @param pThis  Dynamic string.
 * @param nIndex Index where to insert the C string.
 * @param sz     C string to insert.
 */
void elstrInsertCStr(str *pThis, int nIndex, char *sz) {
	if(isNaS(pThis))
		return;

	if(sz == NULL) {
		makeNaS(pThis);
		return;
	}
	size_t nLen = strlen(sz);

	if(nIndex < 0 || nIndex > pThis->nLength) {
		makeNaS(pThis);
		return;
	}

	if(nLen == 0)
		return;

	elstrEnsureCapacity(pThis, pThis->nLength + nLen + 1);
	if(isNaS(pThis))
		return;

	memmove(pThis->szBuf + nIndex + nLen, pThis->szBuf + nIndex, 
		pThis->nLength - nIndex);
	strncpy(pThis->szBuf + nIndex, sz, nLen);

	elstrSetLength(pThis, pThis->nLength + nLen);
}

/**
 * Removes from the dynamic string @e nCount characters starting from the 
 * @e nIndex index.
 * @param pThis  Dynamic string.
 * @param nIndex Position where to start to remove characters.
 * @param nCount Number of characters to remove.
 */
void elstrDelete(str *pThis, int nIndex, size_t nCount) {
	if(isNaS(pThis))
		return;

	if(nIndex < 0 || nIndex > pThis->nLength) {
		makeNaS(pThis);
		return;
	}

	if(nIndex == pThis->nLength || nCount == 0)
		return;

	if(nIndex + nCount > pThis->nLength)
		nCount = pThis->nLength - nIndex;

	memmove(pThis->szBuf + nIndex, pThis->szBuf + nIndex + nCount, 
		pThis->nLength - nIndex - nCount);

	elstrSetLength(pThis, pThis->nLength - nCount);
}

/**
 * Removes from the dynamic string all ocuurences of the specified character.
 * @param pThis Dynamic string.
 * @param ch    Character which occurences need to be removed.
 * @return      Number of removed characters.
 */
size_t elstrDeleteChar(str *pThis, char ch) {
	if(isNaS(pThis))
		return 0;

	if(pThis->nLength == 0)
		return 0;

	size_t nRemoved = 0;
	char *pDest = pThis->szBuf;
	char *pSrc = pDest;
	while(pSrc - pThis->szBuf < pThis->nLength) {
		if(*pSrc != ch) {
			*pDest = *pSrc;
			pDest++;
		} else
			nRemoved++;
		pSrc++;
	}

	elstrSetLength(pThis, pThis->nLength - nRemoved);

	return nRemoved;
}

/**
 * Removes specified leading characters from the dynamic string.
 * @param pThis       Dynamic string.
 * @param arrChars    An array of characters to be removed.
 * @param nCountChars Number of characters in @e arrChars array.
 */
void elstrLTrimChars(str *pThis, char arrChars[], size_t nCountChars) {
	if(isNaS(pThis))
		return;

	if(pThis->nLength == 0 || nCountChars == 0)
		return;

	int nCount = pThis->nLength;
	for(int i = 0; i < pThis->nLength; i++) {
		bool bFound = false;
		for(int j = 0; j < nCountChars; j++)
			if(pThis->szBuf[i] == arrChars[j]) {
				bFound = true;
				break;
			}
		if(!bFound) {
			nCount = i;
			break;
		}
	}

	if(nCount > 0)
		elstrDelete(pThis, 0, nCount);
}

/**
 * Removes leading whitespace (space and tab) characters from the dynamic 
 * string.
 * @param pThis Dynamic string.
 */
void elstrLTrim(str *pThis) {
	elstrLTrimChars(pThis, " \t", 2);
}

/**
 * Removes specified trailing characters from the dynamic string.
 * @param pThis       Dynamic string.
 * @param arrChars    An array of characters to be removed.
 * @param nCountChars Number of characters in @e arrChars array.
 */
void elstrRTrimChars(str *pThis, char arrChars[], size_t nCountChars) {
	if(isNaS(pThis))
		return;

	if(pThis->nLength == 0 || nCountChars == 0)
		return;

	int nStart = 0;
	for(int i = pThis->nLength - 1; i >= 0; i--) {
		bool bFound = false;
		int j;
		for(j = 0; j < nCountChars; j++)
			if(pThis->szBuf[i] == arrChars[j]) {
				bFound = true;
				break;
			}
		if(!bFound) {
			nStart = i + 1;
			break;
		}
	}

	int nCount = pThis->nLength - nStart;

	if(nCount > 0)
		elstrDelete(pThis, nStart, nCount);
}

/**
 * Removes trailing whitespace (space and tab) characters from the dynamic 
 * string.
 * @param pThis Dynamic string.
 */
void elstrRTrim(str *pThis) {
	elstrRTrimChars(pThis, " \t", 2);
}

/**
 * Removes both leading and trailing whitespace (space and tab) characters from 
 * the dynamic string.
 * @param pThis Dynamic string.
 */
void elstrTrim(str *pThis) {
	// Call RTrim first to save little work for LTrim
	elstrRTrim(pThis);
	elstrLTrim(pThis);
}

/**
 * Reverses the dynamic string (makes "dcba" from "abcd").
 * @param pThis Dynamic string.
 */
void elstrReverse(str *pThis) {
	if(isNaS(pThis))
		return;

	if(pThis->nLength == 0)
		return;

	char *p1 = pThis->szBuf;
	char *p2 = pThis->szBuf + pThis->nLength - 1;
	while(p1 < p2) {
		char ch = *p1;
		*p1 = *p2;
		*p2 = ch;
		p1++;
		p2--;
	}
}

/**
 * Compares the dynamic string with a C style string.
 * @param  pThis Dynamic string.
 * @param  sz    C style string to compare with.
 * @return       -1 if dynamic string is less than C style string, 0 if strings
 * are equal, 1 if dynamic string is greater than C style string. If by some 
 * reason it's not possible to compare strings due to errors, returns 
 * EL_STR_ERR_WRONG_STRING or EL_STR_ERR_WRONG_PARAM.
 */
int elstrCompareCStr(str *pThis, const char *sz) {
	if(isNaS(pThis))
		return EL_STR_ERR_WRONG_STRING;

	if(sz == NULL)
		return EL_STR_ERR_WRONG_PARAM;

	size_t nLen = strlen(sz);
	int nMin = pThis->nLength < nLen ? pThis->nLength : nLen;
	int nRes = memcmp(pThis->szBuf, sz, nMin);
	if(nRes == 0) {
		if(pThis->nLength < nLen)
			nRes = -1;
		else
			nRes = pThis->nLength == nLen ? 0 : 1;
	}
	return nRes;
}

/**
 * Checks if the dynamic string is equal to another dynamixc string.
 * @param  pThis Dynamic string.
 * @param  pStr  Dynamic string to check equality with.
 * @return       True if the dynamic string is equal to another one, otherwise 
 * false.
 */
bool elstrIsEqualToELStr(str *pThis, str *pStr) {
	if(isNaS(pThis))
		return false;

	if(pStr == NULL || isNaS(pStr))
		return false;

	if(pThis->nLength != pStr->nLength) 
		return false;

	return memcmp(pThis->szBuf, pStr->szBuf, pStr->nLength) == 0;
}

/**
 * Checks if the dynamic string starts from C string (@e sz). Returns true if 
 * yes, otherwise returns false.
 * @param  pThis Dynamic string.
 * @param  sz    Prefix C style string.
 * @return       True if the dynamic string starts from a specified prefix.
 */
bool elstrHasPrefixCStr(str *pThis, char *sz) {
	if(isNaS(pThis))
		return false;

	if(sz == NULL)
		return false;
	size_t nLen = strlen(sz);

	if(pThis->nLength < nLen) 
		return false;

	return memcmp(pThis->szBuf, sz, nLen) == 0;
}

/**
 * Checks if the dynamic string starts from dynamic string (@e pStr). Returns 
 * true if yes, otherwise returns false.
 * @param  pThis Dynamic string.
 * @param  pStr  Prefix dynamic string.
 * @return       True if the dynamic string starts from a specified prefix.
 */
bool elstrHasPrefixELStr(str *pThis, str *pStr) {
	if(isNaS(pThis))
		return false;

	if(pStr == NULL || isNaS(pStr))
		return false;

	if(pThis->nLength < pStr->nLength) 
		return false;

	return memcmp(pThis->szBuf, pStr->szBuf, pStr->nLength) == 0;
}

/**
 * Checks if the string ends with a specified C string. Returns true if yes, 
 * otherwise returns false.
 * @param  pThis Dynamic string.
 * @param  sz    Suffix C string.
 * @return       True if the dynamic string ends with a specified suffix.
 */
bool elstrHasSuffixCStr(str *pThis, char *sz) {
	if(isNaS(pThis))
		return false;

	if(sz == NULL)
		return false;
	size_t nLen = strlen(sz);

	if(pThis->nLength < nLen) 
		return false;

	return memcmp(pThis->szBuf + pThis->nLength - nLen, sz, nLen) == 0;
}

/**
 * Checks if the string ends with a specified string. Returns true if yes, 
 * otherwise returns false.
 * @param  pThis Dynamic string.
 * @param  pStr  Suffix dynamic string.
 * @return       True if the dynamic string ends with a specified suffix.
 */
bool elstrHasSuffixELStr(str *pThis, str *pStr) {
	if(isNaS(pThis))
		return false;

	if(pStr == NULL || isNaS(pStr))
		return false;

	if(pThis->nLength < pStr->nLength) 
		return false;

	return memcmp(pThis->szBuf + pThis->nLength - pStr->nLength, pStr->szBuf, 
		pStr->nLength) == 0;
}

/**
 * Returns an index of the first occurence of specified character.
 * @param  pThis Dynamic string.
 * @param  ch    Character to search.
 * @return       An index of the character or -1 if character is not found or 
 * error occured.
 */
int elstrIndexOfChar(str *pThis, char ch) {
	if(isNaS(pThis))
		return -1;

	for(int i = 0; i < pThis->nLength; i++)
		if(pThis->szBuf[i] == ch)
			return i;

	return -1;
}

/**
 * Returns an index of the first occurence of specified character starting from 
 * the @e nIndexFrom.
 * @param  pThis      Dynamic string.
 * @param  ch         Character to search.
 * @param  nIndexFrom Position where to start the search.
 * @return            An index of the character or -1 if character is not found 
 * or error occured.
 */
int elstrIndexOfCharFrom(str *pThis, char ch, int nIndexFrom) {
	if(isNaS(pThis))
		return -1;
	if(nIndexFrom < 0 || nIndexFrom > pThis->nLength)
		return -1;

	for(int i = nIndexFrom; i < pThis->nLength; i++)
		if(pThis->szBuf[i] == ch)
			return i;

	return -1;
}

/**
 * Splits the dynamic string by characters from the @e arrChars array. Returns
 * an array of substrings.
 * @param  pThis            Dynamic string.
 * @param  arrChars         An array of characters where to split the dynamic 
 * string.
 * @param  nCountChars      Number of characters in the @e arrChars array.
 * @param  bRemoveEmpty     This flag indicates if emty strings should also be 
 * returned or not.
 * @param  pCountSubstrings Number of generated substrings is returned here.
 * @return                  An array of substrings or NULL is error occured.
 */
str **elstrSplitByChars(str *pThis, char arrChars[], size_t nCountChars, 
	bool bRemoveEmpty, size_t *pCountSubstrings) {

	*pCountSubstrings = 0;

	if(isNaS(pThis))
		return NULL;

	if(pThis->nLength == 0 || nCountChars == 0)
		return NULL;
	
	int nCapacitySubstr = 2;
	str **pSubstr = malloc(sizeof(str*) * nCapacitySubstr);
	if (pSubstr == NULL) 
		return NULL;

	int nCountSubstr = 0;
	int nStart = 0;

	for(size_t i = 0; i < pThis->nLength; i++) {
		bool bSplit = false;
		int j;
		for(j = 0; j < nCountChars; j++)
			if(pThis->szBuf[i] == arrChars[j]) {
				bSplit = true;
				break;
			}

		if(bSplit) {
			if (nCapacitySubstr < nCountSubstr + 2) {
				nCapacitySubstr *= 2;
				str **pSubstrNew = realloc(pSubstr, 
					sizeof(str*) * nCapacitySubstr);
				if (pSubstrNew == NULL) {
					for (int j = 0; j < nCountSubstr; j++) 
						elstrDestroy(pSubstr[j]);
					free(pSubstr);
					return NULL;
				}
				pSubstr = pSubstrNew;
			}			

			if(i - nStart > 0 || !bRemoveEmpty) {
				pSubstr[nCountSubstr] = elstrCreateFromELSubStr(pThis, nStart, 
					i - nStart);
				if(pSubstr[nCountSubstr] == NULL) {
					for (int j = 0; j < nCountSubstr; j++) 
						elstrDestroy(pSubstr[j]);
					free(pSubstr);
					return NULL;
				}
				nCountSubstr++;
			}

			nStart = i + 1;
		}
	}

	if(pThis->nLength - nStart > 0 || !bRemoveEmpty) {
		pSubstr[nCountSubstr] = elstrCreateFromELSubStr(pThis, nStart, 
			pThis->nLength - nStart);
		if(pSubstr[nCountSubstr] == NULL) {
			for (int j = 0; j < nCountSubstr; j++) 
				elstrDestroy(pSubstr[j]);
			free(pSubstr);
			return NULL;
		}
		nCountSubstr++;
	}

	*pCountSubstrings = nCountSubstr;
	return pSubstr;
}

/**
 * Splits the dynamic string to substrings at characters from the @e arrChars
 * array. Returns an array of substrings. Empty substrings are also returned.
 * @param  pThis            Dynamic string.
 * @param  arrChars         An array of characters where to split the dynamic 
 * string.
 * @param  nCountChars      Number of characters in the @e arrChars array.
 * @param  pCountSubstrings Number of generated substrings is returned here.
 * @return                  An array of substrings or NULL is error occured.
 */
str **elstrSplitByCharsNoEmpty(str *pThis, char arrChars[], size_t nCountChars, 
	size_t *pCountSubstrings) {

	return elstrSplitByChars(pThis, arrChars, nCountChars, true, 
		pCountSubstrings);
}

/**
 * Creates an array of N-Grams from the dynamic string. If memory for the array 
 * is allocated by this function it should later be freed by 
 * elstrArrayELStrDestroy().
 * @param  pThis        Dynamic string.
 * @param  nN           Value of N (for N-Gram).
 * @param  pNGramsArr   Pointer to an array of pointers to N-Grams. An array of 
 * N-Grams is returned here. If NULL is passed, an array is not created but 
 * just an amount of memory required is calculated and returned.
 * @param  nSize        If the memory for the array is already allocated, nSize
 * must be equal to buffer size (in bytes) otherwise it should be 0.
 * @param  pCountNGrams Number of generated N-Grams is returned here.
 * @return              Number of bytes used to store all N-Gram strings 
 * (sizeof(str*) + sizeof(str*) + NGram[i]->Capacity) * nCountNGrams). 
 */
size_t elstrMBCreateNGrams(str *pThis, size_t nN, void *pNGramsArr, 
	size_t nSize, size_t *pCountNGrams) {

	*pCountNGrams = 0;

	if(isNaS(pThis))
		return 0;

	if(pThis->nLength == 0 || nN == 0)
		return 0;

	size_t nMBLength = elstrMBGetLength(pThis);
	if(isNaS(pThis))
		return 0;
	if(nMBLength == 0)
		return 0;

	if(nN >= nMBLength)
		*pCountNGrams = 1;
	else
		*pCountNGrams = nMBLength - (nN -1);

	if(nSize > 0 && nSize <= sizeof(str) * *pCountNGrams)
		return 0;

	str **pNGrams = NULL;
	char *szDataBufStart = NULL;
	char *szDataBuf = NULL;
	size_t nDataBufSize = 0;
	str* pStrCur = NULL;
	if(pNGramsArr != NULL) {
		if(nSize == 0) {
			pNGrams = malloc(sizeof(str*) * *pCountNGrams);
			if (pNGrams == NULL) 
				return 0;
			*((str***)pNGramsArr) = pNGrams;
		} else {
			// Buffer layout:
			//   (Pointers to str) * nCountNGrams
			//   str * nCountNGrams
			//   data buffers
			str **pPointers = *((str***)pNGramsArr);
			pStrCur = (str*)((char*)pPointers + sizeof(char*) * *pCountNGrams);
			for(size_t i = 0; i < *pCountNGrams; i++)
				pPointers[i] = &pStrCur[i];
			szDataBufStart = (char*)&pStrCur[*pCountNGrams];
			szDataBuf = szDataBufStart;
			nDataBufSize = nSize - (szDataBufStart - (char*)pPointers);
		}
	}

	size_t nMemTotal = sizeof(str*) * *pCountNGrams;

	mbstate_t mbs;
	// mbrlen(NULL, 0, &mbs); - doesn't work in CentOS 6.5
	memset(&mbs, 0, sizeof(mbstate_t));

	char *szBuf = pThis->szBuf;
	size_t nMax = pThis->nLength;

	char *szNGramStart = szBuf;
	char *szNGramStartNext = NULL;

	size_t nLengthExtra = 0;
	size_t nMBCharsCount = 0;
	size_t nNGramsCount = 0;

	while(nMax != 0) {
		size_t nLengthCur = mbrlen(szBuf, nMax, &mbs);
		
		if(nLengthCur == (size_t)(0) || nLengthCur == (size_t)(-1) || 
			nLengthCur == (size_t)(-2)) {

			if(pNGramsArr != NULL) {
				if(nSize == 0) {
					for (int j = 0; j < nNGramsCount; j++) 
						elstrDestroy(pNGrams[j]);
					free(pNGrams);
				}
			}

			makeNaS(pThis);
			return 0;
		}
		szBuf += nLengthCur;
		nMax -= nLengthCur;
		nMBCharsCount++;

		if(nMBCharsCount == 1) {
			szNGramStartNext = szBuf;
			nLengthExtra = 0;
		} else {
			nLengthExtra += nLengthCur;
		}

		if(nMBCharsCount == nN) {
			size_t nLength = szBuf - szNGramStart;
			if(pNGramsArr != NULL) {
				if(nSize == 0) {
					pNGrams[nNGramsCount] = elstrCreateFromELSubStr(pThis, 
						szNGramStart - pThis->szBuf, nLength);
					if(pNGrams[nNGramsCount] == NULL) {
						for (int j = 0; j < nNGramsCount; j++) 
							elstrDestroy(pNGrams[j]);
						free(pNGrams);

						return 0;
					}
				} else {
					if(szDataBuf - szDataBufStart + nLength + 1 > nDataBufSize)
						return 0;
					elstrCreateEmptyPreallocFixed(&pStrCur[nNGramsCount], 
						szDataBuf, nLength + 1);

					memcpy(pStrCur[nNGramsCount].szBuf, szNGramStart, nLength);
					elstrSetLength(&pStrCur[nNGramsCount], nLength);
					szDataBuf += nLength + 1;
				}
			} 
			nMemTotal += sizeof(str) + nLength + 1;
			nNGramsCount++;

			if(nMax != 0) {
				szBuf = szNGramStartNext;
				nMax += nLengthExtra;
				szNGramStart = szBuf;
			}

			nMBCharsCount = 0;
		}
	}

	if(nMBCharsCount != 0) {
		size_t nLength = szBuf - szNGramStart;
		if(pNGramsArr != NULL) {
			if(nSize == 0) {
				pNGrams[nNGramsCount] = elstrCreateFromELSubStr(pThis, 
					szNGramStart - pThis->szBuf, nLength);
				if(pNGrams[nNGramsCount] == NULL) {
					for (int j = 0; j < nNGramsCount; j++) 
						elstrDestroy(pNGrams[j]);
					free(pNGrams);

					return 0;
				}
			} else {
				if(szDataBuf - szDataBufStart + nLength + 1 > nDataBufSize)
					return 0;
				elstrCreateEmptyPreallocFixed(&pStrCur[nNGramsCount], 
					szDataBuf, nLength + 1);
				memcpy(pStrCur[nNGramsCount].szBuf, szNGramStart, nLength);
				elstrSetLength(&pStrCur[nNGramsCount], nLength);
				// szDataBuf += nLength + 1;
			}
		} 
		nMemTotal += sizeof(str) + nLength + 1;
		// nCountNGrams++;
	}

	return nMemTotal;
}

/**
 * Frees an array of ELStrings previously created by elstrSplitByChars().
 * @param pStrings      An array of ELStrings.
 * @param nCountStrings Number of ELStrings in array.
 */
void elstrArrayELStrDestroy(str **pStrings, size_t nCountStrings) {
	if(pStrings != NULL) {
		for (int i = 0; i < nCountStrings; i++) {
			elstrDestroy(pStrings[i]);
		}
		free(pStrings);
	}
}

/**
 * Returns the maximal number of multibyte characters the dynamic string may 
 * hold.
 * @return Maximal number of multibyte characters the dynamic string may hold.
 */
size_t elstrMBGetMaxLength() {
	return EL_STR_MB_LENGTH_MAX;
}

