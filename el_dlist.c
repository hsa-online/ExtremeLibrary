/* Extreme Library (EL). Doubly linked lists. 
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

#include <stdio.h> 

#include "el_memory.h"

#include "el_dlist.h"

#define isInvalid(s) ((s) == NULL)
#define isInvalidNode(s) ((s) == NULL)
#define isInvalidIterator(s) ((s) == NULL)
#define destroyNode(s, pNode) \
	if((pNode)->pData != NULL && (s)->dataDestructor != NULL) \
		(s)->dataDestructor((pNode)->pData);

dlist_iterator el_dlist_iterator_end = {EL_DIR_FORWARD, NULL};
dlist_iterator el_dlist_iterator_rend = {EL_DIR_BACKWARD, NULL};

/**
 * Creates new doubly linked list node and initializes it with the data 
 * specified.
 * @param  pData Node data.
 * @return       Newly created doubly linked list node (or NULL if an error 
 * occured).
 */
eldlist_node *eldlistNodeCreate(void *pData) {
	eldlist_node *pNode = EL_CALLOC(1, sizeof(eldlist_node));
	
	if(pNode != NULL)
		pNode->pData = pData;

	return pNode;
}

/**
 * Destroys the doubly linked list node and frees its data. 
 * @param pThis  Doubly linked list node to be destroyed.
 * @param pDList Doubly linked list responsible to free node's data.
 */
void eldlistNodeDestroy(eldlist_node *pThis, dlist *pDList) {
	if(isInvalidNode(pThis))
		return;

	if(isInvalid(pDList))
		return;

	destroyNode(pDList, pThis);

	EL_FREE(pThis);
}

/**
 * Creates new doubly linked list iterator.
 * @param  nDirection Iterator's direction.
 * @param  pNode      Starting node.
 * @return            Newly created doubly linked list iterator (or NULL if an 
 * error occured).
 */
dlist_iterator *eldlistIteratorCreate(el_direction nDirection, 
	eldlist_node *pNode) {

	dlist_iterator *pThis = EL_CALLOC(1, sizeof(dlist_iterator));
	if(pThis == NULL)
		return NULL;

	pThis->nDirection = nDirection;
	pThis->pNode = pNode;

	return pThis;
}

/**
 * Destroys the doubly linked list iterator.
 * @param pThis Doubly linked list iterator to be destroyed.
 */
void eldlistIteratorDestroy(dlist_iterator *pThis) {
	if(isInvalidIterator(pThis))
		return;

	EL_FREE(pThis);
}

/**
 * Checks if two specified iterators are equal.
 * @param  pIterator1 First iterator.
 * @param  pIterator2 Second iterator.
 * @return            True if iterators are equal.
 */
extern bool eldlistIteratorsAreEqual(dlist_iterator *pIterator1, 
	dlist_iterator *pIterator2);

/**
 * Moves iterator to the next item of doubly linked list.
 * @param  pThis Doubly linked list iterator.
 * @return       True if operation was successful.
 */
bool eldlistIteratorNext(dlist_iterator *pThis) {
	if(isInvalidIterator(pThis))
		return false;
	if(pThis->pNode == NULL)
		return false;

	switch(pThis->nDirection) {
		case EL_DIR_FORWARD:
			pThis->pNode = pThis->pNode->pNext;
			return true;
		case EL_DIR_BACKWARD:
			pThis->pNode = pThis->pNode->pPrev;
			return true;
		default:
			// nDirection seems to be invalid
			return false;
	}
}

/**
 * Returns the data associated with the current item of the doubly linked list 
 * iterated.
 * @param  pThis Doubly linked list iterator.
 * @return       Current item's data or NULL if an item has no data or an error 
 * occured.
 */
void *eldlistIteratorGetData(dlist_iterator *pThis) {
	if(isInvalidIterator(pThis))
		return NULL;
	if(pThis->pNode == NULL)
		return NULL;

	return pThis->pNode->pData;
}

/**
 * Creates new empty doubly linked list.
 * @param  dataDestructor Pointer to callback function which will be called for 
 * each item to destroy it.
 * @param  dataComparer   Pointer to callback function which will be called to 
 * compare item data.
 * @return                Newly created doubly linked list (or NULL if an error 
 * occured).
 */
dlist *eldlistCreate(void (*dataDestructor)(void *pData),
	bool (*dataComparer)(void *p1, void *p2)) {

	dlist *pThis = EL_CALLOC(1, sizeof(dlist));
	if(pThis == NULL)
		return NULL;

	pThis->dataDestructor = dataDestructor;
	pThis->dataComparer = dataComparer;

	return pThis;
}

/**
 * Destroys all nodes of doubly linked list. Doesn't change the state of the 
 * list. Should be called internally only.
 * @param  pThis Doubly linked list.
 */
void eldlistAllNodesDestroy(dlist *pThis) {
	if(isInvalid(pThis))
		return;

	eldlist_node *pNode = pThis->pHead;

	while(pNode != NULL) {
		pThis->pHead = pThis->pHead->pNext;

		destroyNode(pThis, pNode);

		pNode = pThis->pHead;
	}
}

/**
 * Destroys the doubly linked list.
 * @param pThis Doubly linked list to be destroyed.
 */
void eldlistDestroy(dlist *pThis) {
	if(isInvalid(pThis))
		return;

 	eldlistAllNodesDestroy(pThis);

	EL_FREE(pThis);
}

/**
 * Returns number of nodes in the list.
 * @param  pThis Doubly linked list.
 * @return       Number of nodes in the list.
 */
size_t elstrGetCount(dlist *pThis)  {
	if(isInvalid(pThis))
		return 0;

	return pThis->nCount;
}


/**
 * Clears the doubly linked list.
 * @param  pThis Doubly linked list.
 * @return       True if operation was successful.
 */
bool eldlistClear(dlist *pThis) {
	if(isInvalid(pThis))
		return false;

	eldlistAllNodesDestroy(pThis);

	pThis->pHead = NULL;
	pThis->pTail = NULL;
	pThis->nCount = 0;

	return true;
}

/**
 * Adds new node at the beggining of doubly linked list.
 * @param  pThis Doubly linked list.
 * @param  pNode The node to be added.
 * @return       Added node (or NULL if an error occured).
 */
eldlist_node *eldlistAddFirstNode(dlist *pThis, eldlist_node *pNode) {
	if(isInvalid(pThis) || pNode == NULL)
		return NULL;

	if(pThis->nCount > 0) {
		pNode->pPrev = NULL;
		pNode->pNext = pThis->pHead;
		pThis->pHead->pPrev = pNode;
		pThis->pHead = pNode;
	} else {
		pThis->pHead = pNode;
		pThis->pTail = pNode;
	}
	pThis->nCount++;

	return pNode;
}

/**
 * Adds the data at the beggining of doubly linked list.
 * @param  pThis Doubly linked list.
 * @param  pData Data to be added to the list
 * @return       Newly created node (or NULL if an error occured).
 */
eldlist_node *eldlistAddFirst(dlist *pThis, void *pData) {
	if(isInvalid(pThis))
		return NULL;

	return eldlistAddFirstNode(pThis, eldlistNodeCreate(pData));
}

/**
 * Adds new node at the end of doubly linked list.
 * @param  pThis Doubly linked list.
 * @param  pNode The node to be added.
 * @return       Added node (or NULL if an error occured).
 */
eldlist_node *eldlistAddLastNode(dlist *pThis, eldlist_node *pNode) {
	if(isInvalid(pThis) || pNode == NULL)
		return NULL;

	if(pThis->nCount > 0) {
		pNode->pPrev = pThis->pTail;
		pNode->pNext = NULL;
		pThis->pTail->pNext = pNode;
		pThis->pTail = pNode;
	} else {
		pThis->pHead = pNode;
		pThis->pTail = pNode;
	}
	pThis->nCount++;

	return pNode;
}

/**
 * Adds the data at the end of doubly linked list.
 * @param  pThis Doubly linked list.
 * @param  pData Data to be added to the list
 * @return       Newly created node (or NULL if an error occured).
 */
eldlist_node *eldlistAddLast(dlist *pThis, void *pData) {
	if(isInvalid(pThis))
		return NULL;

	return eldlistAddLastNode(pThis, eldlistNodeCreate(pData));
}

/**
 * Returns a first node of doubly linked list.
 * @param  pThis Doubly linked list.
 * @return       First node. Returns NULL if the list is empty or an error 
 * occured.
 */
eldlist_node *eldlistGetFirstNode(dlist *pThis) {
	if(isInvalid(pThis))
		return NULL;

	return pThis->pHead;
}

/**
 * Returns a last node of doubly linked list.
 * @param  pThis Doubly linked list.
 * @return       Last node. Returns NULL if the list is empty or an error 
 * occured.
 */
eldlist_node *eldlistGetLastNode(dlist *pThis) {
	if(isInvalid(pThis))
		return NULL;

	return pThis->pTail;
}

/**
 * Searches for the first node containing the data specified.
 * <br> Complexity of this function is O(n).
 * @param  pThis Doubly linked list.
 * @param  pData Node data.
 * @return       First node containing the data specified. Returns NULL if the 
 * node is not found or an error occured.
 */
eldlist_node *eldlistSearch(dlist *pThis, void *pData) {
	if(isInvalid(pThis))
		return NULL;

	eldlist_node *pNode = pThis->pHead;
	if(pThis->dataComparer != NULL) {
		while(pNode != NULL) {
			if(pThis->dataComparer(pNode->pData, pData))
				break;
			pNode = pNode->pNext;
		}
	} else
		while(pNode != NULL) {
			if(pNode->pData == pData)
				break;
			pNode = pNode->pNext;
		}

	return pNode;
}

/**
 * Searches for the first node containing the data specified and removes it from 
 * the list.
 * @param  pThis Doubly linked list.
 * @param  pData The data of node to be removed.
 * @return       True if node was actually removed.
 */
bool eldlistRemove(dlist *pThis, void *pData) {
	eldlist_node *pNode = eldlistSearch(pThis, pData);
	
	if(pNode != NULL) {
		return eldlistRemoveNode(pThis, pNode);
	} else
		return false;
}

/**
 * Removes the node specified from the list.
 * @param  pThis Doubly linked list.
 * @param  pNode Node to be removed.
 * @return       True if node was actually removed.
 */
bool eldlistRemoveNode(dlist *pThis, eldlist_node *pNode) {
	if(isInvalid(pThis))
		return false;

	if(pNode == NULL)
		return false;

	if(pThis->nCount == 1) {
		destroyNode(pThis, pNode);

		pThis->pHead = NULL;
		pThis->pTail = NULL;
	} else
		if(pNode == pThis->pHead) {
			pThis->pHead = pThis->pHead->pNext;
			pThis->pHead->pPrev = NULL;

			destroyNode(pThis, pNode);
		} else
			if(pNode == pThis->pTail) {
				pThis->pTail = pThis->pTail->pPrev;
				pThis->pTail->pNext = NULL;

				destroyNode(pThis, pNode);
			} else {
				pNode->pNext->pPrev = pNode->pPrev;
				pNode->pPrev->pNext = pNode->pNext;

				destroyNode(pThis, pNode);
			}

	pThis->nCount--;

	return true;
}

/**
 * Returns an iterator pointing to the first element of doubly linked list.
 * @param  pThis Doubly linked list.
 * @return       An iterator to the beginning of doubly linked list.
 */
extern dlist_iterator *eldlistBegin(dlist *pThis);

/**
 * Returns an iterator referring to the past-the-end element of doubly linked 
 * list.
 * @param  pThis Doubly linked list.
 * @return       An iterator to the element past the end of the doubly linked 
 * list.
 */
extern dlist_iterator *eldlistEnd(dlist *pThis);

/**
 * Returns a reverse iterator pointing to the last element of doubly linked 
 * list.
 * @param  pThis Doubly linked list.
 * @return       An iterator to the end of doubly linked list.
 */
extern dlist_iterator *eldlistRBegin(dlist *pThis);

/**
 * Returns an iterator referring to the element prior to the first elemet of 
 * doubly linked list.
 * @param  pThis Doubly linked list.
 * @return       An iterator to the element prior to the first elemet of the 
 * doubly linked list.
 */
extern dlist_iterator *eldlistREnd(dlist *pThis);

/**
 * Iterates through the doubly linked list and calls specified function for each 
 * item of the list. If function returns @b false - stops iteration.
 * @param pThis        Doubly linked list.
 * @param dataCallback Callback function to be called for each item of the list.
 */
void eldlistForEach(dlist *pThis, bool (*dataCallback)(void *pData)) {
	if(isInvalid(pThis))
		return;

	if(dataCallback == NULL)
		return;

	dlist_iterator *it;
	for(it = eldlistBegin(pThis); 
		!eldlistIteratorsAreEqual(it, eldlistEnd(pThis)); 
		eldlistIteratorNext(it)) {
		
		if(!dataCallback(eldlistIteratorGetData(it)))
			break;
	}
	eldlistIteratorDestroy(it);
}

/**
 * Iterates through the doubly linked list and calls specified function for each 
 * item of the list. Passes the pointer to custom data to the callback function.
 * If function returns @b false - stops iteration.
 * @param pThis        Doubly linked list.
 * @param dataCallback Callback function to be called for each item of the list.
 * @param pEx          Pointer to custom data to be sent to callback.
 */
void eldlistForEachEx(dlist *pThis, 
	bool (*dataCallbackEx)(void *pData, void *pEx), void *pEx) {

	if(isInvalid(pThis))
		return;

	if(dataCallbackEx == NULL)
		return;

	dlist_iterator *it;
	for(it = eldlistBegin(pThis); 
		!eldlistIteratorsAreEqual(it, eldlistEnd(pThis)); 
		eldlistIteratorNext(it)) {
		
		if(!dataCallbackEx(eldlistIteratorGetData(it), pEx))
			break;
	}
	eldlistIteratorDestroy(it);
}
