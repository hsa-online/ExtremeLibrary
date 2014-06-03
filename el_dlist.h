/* Extreme Library (EL). Double linked lists. 
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

#ifndef _EL_DLIST_H_
#define _EL_DLIST_H_

#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	EL_DIR_FORWARD,
	EL_DIR_BACKWARD
} el_direction;

/** 
 * @brief Holds the data of double linked list  node.
 */
typedef struct eldlist_node {
	struct eldlist_node *pPrev; /**< Pointer to previous node. */
	struct eldlist_node *pNext; /**< Pointer to next node. */
	void *pData; /**< Pointer to the data of this node. */
} eldlist_node;

/** 
 * @brief Double linked list.
 */
typedef struct dlist {
	eldlist_node *pHead; /**< Pointer to head of the list. */
	eldlist_node *pTail; /**< Pointer to tail of the list. */
	size_t nCount; /**< Number of elements in the list. */
	void (*dataDestructor)(void *pData); /**< Pointer to callback which 
	destroys data. */
	bool (*dataComparer)(void *p1, void *p2); /**< Pointer to callback which 
	compares 2 data items. */
} dlist;

/** 
 * @brief Double linked list iterator.
 */
typedef struct dlist_iterator {
	el_direction nDirection; /**< Iterator direction. */
	eldlist_node *pNode; /**< Pointer to current node. */
} dlist_iterator;

extern dlist_iterator el_dlist_iterator_end;
extern dlist_iterator el_dlist_iterator_rend;

/** 
 * @brief Pointer to callback function which destroys the data.
 */
#define EL_CB_DATA_DESTRUCTOR(s) (void (*)(void *))(s)
/** 
 * @brief Pointer to callback function which compares item data.
 */
#define EL_CB_DATA_COMPARER(s) (bool (*)(void *, void *))(s)
/** 
 * @brief Pointer to callback function which's called by foreach() enumerator.
 */
#define EL_CB_FOREACH(s) (bool (*)(void *))(s)

eldlist_node *eldlistNodeCreate(void *pData);
void eldlistNodeDestroy(eldlist_node *pThis, dlist *pDList);

dlist_iterator *eldlistIteratorCreate(el_direction nDirection, 
	eldlist_node *pNode);
void eldlistIteratorDestroy(dlist_iterator *pThis);
inline bool eldlistIteratorsAreEqual(dlist_iterator *pIterator1, 
	dlist_iterator *pIterator2) {

	return pIterator1->nDirection == pIterator2->nDirection && 
		pIterator1->pNode == pIterator2->pNode;
}
bool eldlistIteratorNext(dlist_iterator *pThis);
void *eldlistIteratorGetData(dlist_iterator *pThis);

dlist *eldlistCreate(void (*dataDestructor)(void *pData), 
	bool (*dataComparer)(void *p1, void *p2));
void eldlistDestroy(dlist *pThis);
size_t elstrGetCount(dlist *pThis);
bool eldlistClear(dlist *pThis);
eldlist_node *eldlistAddFirst(dlist *pThis, void *pData);
eldlist_node *eldlistAddFirstNode(dlist *pThis, eldlist_node *pNode);
eldlist_node *eldlistAddLast(dlist *pThis, void *pData);
eldlist_node *eldlistAddLastNode(dlist *pThis, eldlist_node *pNode);
eldlist_node *eldlistGetFirstNode(dlist *pThis);
eldlist_node *eldlistGetLastNode(dlist *pThis);
eldlist_node *eldlistSearch(dlist *pThis, void *pData);
bool eldlistRemove(dlist *pThis, void *pData);
bool eldlistRemoveNode(dlist *pThis, eldlist_node *pNode);
inline dlist_iterator *eldlistBegin(dlist *pThis) {
	return eldlistIteratorCreate(EL_DIR_FORWARD, pThis->pHead);
}
inline dlist_iterator *eldlistEnd(dlist *pThis)  {
	return &el_dlist_iterator_end;
}
inline dlist_iterator *eldlistRBegin(dlist *pThis) {
	return eldlistIteratorCreate(EL_DIR_BACKWARD, pThis->pTail);
}
inline dlist_iterator *eldlistREnd(dlist *pThis)  {
	return &el_dlist_iterator_rend;
}
void eldlistForEach(dlist *pThis, bool (*dataCallback)(void *pData));

#ifdef __cplusplus
}
#endif

#endif
