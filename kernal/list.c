/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "list.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief To check if the target node contained in the provided list.
 *
 * To check if the target node contained in the provided list.
 * The pointer of list and node must not be an NULL, if the node meet
 * in the list return true.
 *
 * @param pList The pointer of the list.
 * @param pNode The pointer of the node.
 *
 * @return The true indicates the node is existed, otherwist is not existed.
 */
b_t list_node_isExisted(list_t *pList, list_node_t *pNode)
{
    if ((!pList) || (!pNode)) {
        return FALSE;
    }

    list_node_t *pCurNode_temp = pList->pHead;

    while (pCurNode_temp) {
        if (pCurNode_temp == pNode) {
            return TRUE;
        }
        pCurNode_temp = pCurNode_temp->pNext;
    }
    return FALSE;
}

/**
 * @brief To calculate the total number of node contained in the provided list.
 *
 * To calculate the total number of node contained in the provided list, and
 * to return the total number
 *
 * @param pList The pointer of the list.
 *
 * @return Value The total number of node
 */
u32_t list_size(list_t *pList)
{
    if (!pList) {
        return 0u;
    }

    u32_t size = 0;
    list_node_t *pCurNode_temp = pList->pHead;
    while (pCurNode_temp) {
        pCurNode_temp = pCurNode_temp->pNext;
        size++;
    }
    return size;
}

/**
 * @brief To delete a node form the provided list.
 *
 * To delete a node form the provided list, and return the value to indicate
 * the process result
 *
 * @param pList The pointer of the list.
 * @param pNode The pointer of the node.
 *
 * @return The value true indicates the process of removing a node is successful, otherwise is failed.
 */
b_t list_node_delete(list_t *pList, list_node_t *pTargetNode)
{
    if ((!pList) || (!pTargetNode)) {
        return FALSE;
    }

    list_node_t *pCurNode_tmp = pList->pHead;
    list_node_t *pPrevNode_tmp = NULL;
    while ((pCurNode_tmp) && (pCurNode_tmp != pTargetNode)) {
        pPrevNode_tmp = pCurNode_tmp;
        pCurNode_tmp = pCurNode_tmp->pNext;
    }

    if (!pCurNode_tmp) {
        /* The target pNode is not in the list */
        return FALSE;
    }

    if (!pPrevNode_tmp) {
        pList->pHead = pCurNode_tmp->pNext;
    } else {
        pPrevNode_tmp->pNext = pCurNode_tmp->pNext;
    }
    pCurNode_tmp->pNext = NULL;

    return TRUE;
}

/**
 * @brief To insert a node before of a target node in the list.
 *
 * To insert a node before of a target node in the list, and to return the result.
 *
 * @param pList The pointer of the list.
 * @param pBefore The pointer of the before node.
 * @param pTargetNode The pointer of the target node.
 *
 * @return The value true indicates the process is successful, otherwise is failed.
 */
b_t list_node_insertBefore(list_t *pList, list_node_t *pBefore, list_node_t *pTargetNode)
{
    if ((!pList) || (!pBefore) || (!pTargetNode)) {
        return FALSE;
    }

    list_node_t *pCurNode_tmp = pList->pHead;
    list_node_t *pPrevNode_tmp = NULL;
    while ((pCurNode_tmp) && (pCurNode_tmp != pBefore)) {
        pPrevNode_tmp = pCurNode_tmp;
        pCurNode_tmp = pCurNode_tmp->pNext;
    }

    if (!pCurNode_tmp) {
        /* The pBefore is not in the list */
        return FALSE;
    }

    if (!pPrevNode_tmp) {
        pList->pHead = pTargetNode;
    } else {
        pPrevNode_tmp->pNext = pTargetNode;
    }
    pTargetNode->pNext = pBefore;

    return TRUE;
}

/**
 * @brief To push a node based on direction.
 *
 * To push a node based on direction, and to return the result.
 *
 * @param pList The pointer of the list.
 * @param pInNode The pointer of the pushed node.
 * @param direction The direction of list
 *
 * @return The value true indicates the process is successful, otherwise is failed.
 */
b_t list_node_push(list_t *pList, list_node_t *pInNode, list_direction_t direction)
{
    if ((!pList) || (!pInNode) || ((direction != LIST_HEAD) && (direction != LIST_TAIL))) {
        return FALSE;
    }

    if (direction == LIST_TAIL) {
        list_node_t *pCurNode_temp = pList->pHead;
        if (pCurNode_temp) {
            while (pCurNode_temp->pNext) {
                pCurNode_temp = pCurNode_temp->pNext;
            }
            pCurNode_temp->pNext = pInNode;
        } else {
            pList->pHead = pInNode;
        }
        pInNode->pNext = NULL;
    } else if (direction == LIST_HEAD) {
        pInNode->pNext = pList->pHead;
        pList->pHead = pInNode;
    }

    return TRUE;
}

/**
 * @brief To pop a node from the provided list based on direction.
 *
 * To pop a node based on direction, and to return the result.
 *
 * @param pList The pointer of the list.
 * @param direction The direction of list.
 *
 * @return The value is node pointer, but the null indicates there is no available node to pop.
 */
list_node_t *list_node_pop(list_t *pList, list_direction_t direction)
{
    if ((!pList) || ((direction != LIST_HEAD) && (direction != LIST_TAIL))) {
        return NULL;
    }

    list_node_t *pOutNode = NULL;
    if (direction == LIST_TAIL) {
        list_node_t *pCurNode_temp = pList->pHead;
        list_node_t *pPrevNode_tmp = NULL;
        if (pCurNode_temp) {
            while (pCurNode_temp->pNext) {
                pPrevNode_tmp = pCurNode_temp;
                pCurNode_temp = pCurNode_temp->pNext;
            }

            pOutNode = pCurNode_temp;

            if (pPrevNode_tmp) {
                pPrevNode_tmp->pNext = NULL;
            } else {
                pList->pHead = NULL;
            }
        }

        return pOutNode;
    }

    if (direction == LIST_HEAD) {
        pOutNode = pList->pHead;
        if (pList->pHead) {
            pList->pHead = pList->pHead->pNext;
            pOutNode->pNext = NULL;
        }

        return pOutNode;
    }

    return NULL;
}

/**
 * @brief Initialize a iterator to traverse all node in the list from the list head.
 *
 * Initialize a iterator to traverse all node in the list, and to return the result.
 *
 * @param pIterator The pointer of the iterator.
 * @param pList The pointer of the list.
 *
 * @return The true indicates the iterator symbol created successful, otherwist is failed.
 */
b_t list_iterator_init(list_iterator_t *pIterator, list_t *pList)
{
    if ((!pIterator) || (!pList)) {
        return FALSE;
    }

    _memset((char_t *)pIterator, 0x0u, sizeof(list_iterator_t));
    if (pList->pHead) {
        pIterator->pCurNode = pList->pHead;
        pIterator->pList = pList;

        return TRUE;
    }

    return FALSE;
}

/**
 * @brief Initialize a iterator to traverse all node in the list to output a node.
 *
 * Initialize a iterator to traverse all node in the list to output a node, and to return the result.
 *
 * @param pIterator The pointer of the iterator.
 *
 * @return The next node pointer in the list.
 */
list_node_t *list_iterator_next(list_iterator_t *pIterator)
{
    if (!pIterator) {
        return NULL;
    }

    list_node_t *pCurOutNode = pIterator->pCurNode;
    if (pIterator->pCurNode) {
        pIterator->pCurNode = pIterator->pCurNode->pNext;
    }

    return pCurOutNode;
}

/**
 * @brief Initialize a iterator to traverse all node in the list to output a node and a result.
 *
 * Initialize a iterator to traverse all node in the list to output a node and a result, and to return the result.
 *
 * @param pIterator The pointer of the iterator.
 * @param ppOutNode The double pointer of the output node.
 *
 * @return The true indicates the output node is not NULL.
 */
b_t list_iterator_next_condition(list_iterator_t *pIterator, list_node_t **ppOutNode)
{
    *ppOutNode = list_iterator_next(pIterator);

    return (b_t)((*ppOutNode) ? TRUE : FALSE);
}

#ifdef __cplusplus
}
#endif
