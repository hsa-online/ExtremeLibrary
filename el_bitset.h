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

#ifndef _EL_BITSET_H_
#define _EL_BITSET_H_

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** 
 * @brief Holds the data required to represent a bit set data structure.
 *
 * Dictionary is based on the hash table.
 */
typedef struct bitset {
	size_t nCapacity; /**< Number of elements in data buffer. */
	size_t nBitsPerElement; /**< Number of bits in each element of the data 
	buffer. */
	uintmax_t *pBuf; /**< Data buffer. */
} bitset;

bitset *elbitsetCreate(size_t nBitCapacityMin);
void elbitsetDestroy(bitset *pThis);
size_t elbitsetGetSize(bitset *pThis);
size_t elbitsetGetCount(bitset *pThis);
void elbitsetSet(bitset *pThis, size_t nPos);
void elbitsetSetAll(bitset *pThis);
void elbitsetReset(bitset *pThis, size_t nPos);
void elbitsetResetAll(bitset *pThis);
void elbitsetFlip(bitset *pThis, size_t nPos);
void elbitsetFlipAll(bitset *pThis);
bool elbitsetTest(bitset *pThis, size_t nPos);
bool elbitsetAny(bitset *pThis);
bool elbitsetNone(bitset *pThis);
bool elstrIsEqualTo(bitset *pThis, bitset *pOther);
bool elstrIsEqualToEx(bitset *pThis, bitset *pOther);

#ifdef __cplusplus
}
#endif

#endif
