/* Extreme Library (EL). Bit sets. 
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

#include <string.h>

#include "el_memory.h"

#include "el_bitset.h"

#define isInvalid(s) ((s) == NULL)

/**
 * Creates new empty bit set with the capacity not smaller than the specified 
 * one. C
 * @param  nBitCapacityMin Mimimal capacity required.
 * @return                 Newly created bit set (or NULL if an error occured).
 */
bitset *elbitsetCreate(size_t nBitCapacityMin) {
	if(nBitCapacityMin < 1)
		return NULL;

	bitset *pThis = EL_CALLOC(1, sizeof(bitset));
	if(pThis == NULL)
		return NULL;

	pThis->nBitsPerElement = sizeof(uintmax_t) << 3;
	pThis->nCapacity = (nBitCapacityMin + pThis->nBitsPerElement - 1) / 
		pThis->nBitsPerElement;

	pThis->pBuf = EL_CALLOC(1, pThis->nCapacity * sizeof(uintmax_t));
	if(pThis->pBuf == NULL) {
		EL_FREE(pThis);
		return NULL;
	}

	return pThis;
}

/**
 * Destroys the bit set.
 * @param pThis Bit set string to be destroyed.
 */
void elbitsetDestroy(bitset *pThis) {
	if(isInvalid(pThis))
		return;

	EL_FREE(pThis);
}

/**
 * Returns the total number of bits this bit set may hold.
 * @param  pThis Bit set.
 * @return       Number of bits this bit set may hold.
 */
size_t elbitsetGetSize(bitset *pThis) {
	if(isInvalid(pThis))
		return 0;

	return pThis->nBitsPerElement * pThis->nCapacity;
}

/**
 * Returns the number of bits in the bit set which are set.
 * @param  pThis Bit set.
 * @return       Number of bits in this bit set which are set.
 */
size_t elbitsetGetCount(bitset *pThis) {
	if(isInvalid(pThis))
		return 0;

	size_t nCount = 0;
	uintmax_t *pEnd = pThis->pBuf + pThis->nCapacity;
	for(uintmax_t *pCur = pThis->pBuf; pCur < pEnd; pCur++) {
		uintmax_t nV = *pCur;
		for (; nV; nCount++)
			nV &= nV - 1; // clear the least significant bit set
	}

	return nCount;
}

/**
 * Sets the bit at the specified position.
 * @param pThis Bit set.
 * @param nPos  Position of the bit.
 */
void elbitsetSet(bitset *pThis, size_t nPos) {
	if(isInvalid(pThis))
		return;

	size_t nIndex = nPos / pThis->nBitsPerElement;
	if(nIndex >= pThis->nCapacity)
		return;

	uintmax_t nValue = 1;
	nValue <<= (nPos % pThis->nBitsPerElement);

	pThis->pBuf[nIndex] |= nValue;
}

/**
 * Sets all bits of the bit set.
 * @param pThis Bit set.
 */
void elbitsetSetAll(bitset *pThis) {
	if(isInvalid(pThis))
		return;

	memset(pThis->pBuf, 0xFF, pThis->nCapacity * sizeof(uintmax_t));	
}

/**
 * Clears the bit at the specified position.
 * @param pThis Bit set.
 * @param nPos  Position of the bit.
 */
void elbitsetReset(bitset *pThis, size_t nPos) {
	if(isInvalid(pThis))
		return;

	size_t nIndex = nPos / pThis->nBitsPerElement;
	if(nIndex >= pThis->nCapacity)
		return;

	uintmax_t nValue = 1;
	nValue <<= (nPos % pThis->nBitsPerElement);

	pThis->pBuf[nIndex] &= ~nValue;
}

/**
 * Clears all bits of the bit set.
 * @param pThis Bit set.
 */
void elbitsetResetAll(bitset *pThis) {
	if(isInvalid(pThis))
		return;

	memset(pThis->pBuf, 0, pThis->nCapacity * sizeof(uintmax_t));	
}

/**
 * Flips value of specified bit converting zero into one and one into zero.
 * @param pThis [description]
 * @param nPos  Position of the bit.
 */
void elbitsetFlip(bitset *pThis, size_t nPos) {
	size_t nIndex = nPos / pThis->nBitsPerElement;
	if(nIndex >= pThis->nCapacity)
		return;

	uintmax_t nValue = 1;
	nValue <<= (nPos % pThis->nBitsPerElement);

	pThis->pBuf[nIndex] ^= nValue;
}

/**
 * Flips values of all bits converting zeros into ones and ones into zeros.
 * @param pThis Bit set.
 */
void elbitsetFlipAll(bitset *pThis) {
	uintmax_t nValue = 0;
	nValue = ~nValue;

	uintmax_t *pEnd = pThis->pBuf + pThis->nCapacity;
	for(uintmax_t *pCur = pThis->pBuf; pCur < pEnd; pCur++)
		*pCur ^= nValue;
}

/**
 * Checks if the bit at specified position is set.
 * @param pThis Bit set.
 * @param nPos  Position of the bit.
 */
bool elbitsetTest(bitset *pThis, size_t nPos) {
	if(isInvalid(pThis))
		return false;

	size_t nIndex = nPos / pThis->nBitsPerElement;
	if(nIndex >= pThis->nCapacity)
		return false;

	uintmax_t nValue = 1;
	nValue <<= (nPos % pThis->nBitsPerElement);

	return (pThis->pBuf[nIndex] & nValue) != (uintmax_t)0;
}

/**
 * Checks if any of the bits is set.
 * @param  pThis Bit set.
 * @return       @b True if any of the bits is set, otherwise @b false.
 */
bool elbitsetAny(bitset *pThis) {
	if(isInvalid(pThis))
		return false;

	uintmax_t *pEnd = pThis->pBuf + pThis->nCapacity;
	for(uintmax_t *pCur = pThis->pBuf; pCur < pEnd; pCur++)
		if(*pCur != 0)
			return true;

	return false;
}

/**
 * Checks if none of the bits is set.
 * @param  pThis Bit set.
 * @return       @b True if none of the bits is set, otherwise @b false.
 */
bool elbitsetNone(bitset *pThis) {
	return !elbitsetAny(pThis);
}

/**
 * Checks if the bit set is equal to another bit set. If bit sets have different 
 * size they are not equal.
 * @param  pThis  Bit set.
 * @param  pOther Bit set to check equality with.
 * @return        @b True if the bit set has same bits set as another bit set, 
 * otherwise @b false.
 */
bool elstrIsEqualTo(bitset *pThis, bitset *pOther) {
	if(isInvalid(pThis))
		return false;

	if(isInvalid(pOther))
		return false;

	if(pThis->nCapacity == pOther->nCapacity) {
		return memcmp(pThis->pBuf, pOther->pBuf, 
			pThis->nCapacity * sizeof(uintmax_t)) == 0;
	} else
		return false;
}

/**
 * Extended equal. Checks if the bit set is equal to another bit set. Bit sets 
 * can have different size. In this case all high bits of "longerst" bit set 
 * which are equal zero don't affect on the result of comparison. <br>
 * For example: bit sets "00000100" and "100" are equal, but bit sets
 * "00010100" and "100" aren't (leftmost bit is highest, rightmost - lowest).
 * @param  pThis  Bit set.
 * @param  pOther Bit set to check equality with.
 * @return        @b True if the bit set has same bits set as another bit set, 
 * otherwise @b false.
 */
bool elstrIsEqualToEx(bitset *pThis, bitset *pOther) {
	if(isInvalid(pThis))
		return false;

	if(isInvalid(pOther))
		return false;

	if(pThis->nCapacity == pOther->nCapacity) {
		return memcmp(pThis->pBuf, pOther->pBuf, 
			pThis->nCapacity * sizeof(uintmax_t)) == 0;
	} else {
		size_t nCapacityMin;
		uintmax_t *pCur;
		uintmax_t *pEnd;

		if(pThis->nCapacity < pOther->nCapacity) {
			nCapacityMin = pThis->nCapacity;
			pCur = pOther->pBuf + nCapacityMin;
			pEnd = pOther->pBuf + pOther->nCapacity;
		} else {
			nCapacityMin = pOther->nCapacity;
			pCur = pThis->pBuf + nCapacityMin;
			pEnd = pThis->pBuf + pThis->nCapacity;
		}

		if(memcmp(pThis->pBuf, pOther->pBuf, 
			nCapacityMin * sizeof(uintmax_t)) != 0)
			return false;
		// If at least one bit after the end of smallest buffer is set, bit sets 
		//   are different.
		for(; pCur < pEnd; pCur++)
			if(*pCur != 0)
				return false;
	}
	return true;
}
