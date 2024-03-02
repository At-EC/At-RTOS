/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
**/

#include "linker.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief linker node transaction from a list to another list.
 *
 * @param pLinker The pointer of the linker.
 * @param pToList The pointer of the target list.
 * @param direction The direction of list
 */
void linker_list_transaction_common(linker_t *pLinker, list_t *pToList, list_direction_t direction)
{
    if (!pLinker) {
        return;
    }

    if ((direction != LIST_HEAD) && (direction != LIST_TAIL)) {
        return;
    }

    /* Remove the node from the previous list */
    if (pLinker->pList) {
        list_node_delete(pLinker->pList, &pLinker->node);
    }

    if (pToList) {
        list_node_push(pToList, (list_node_t *)&pLinker->node, direction);
    }
    pLinker->pList = pToList;
}

/**
 * @brief linker node transaction from a list to another list with specific condition.
 *
 * @param pLinker The pointer of the linker.
 * @param pToList The pointer of the target list.
 * @param pConditionFunc The pointer of the condition function.
 */
void linker_list_transaction_specific(linker_t *pLinker, list_t *pToList, pLinkerSpecificConditionFunc_t pConditionFunc)
{
    if (!pLinker) {
        return;
    }

    if (!pToList) {
        return;
    }

    if (!pConditionFunc) {
        return;
    }

    /* Remove the node from the previous list */
    if (pLinker->pList) {
        list_node_delete(pLinker->pList, &pLinker->node);
    }

    pLinker->pList = pToList;

    list_node_t *pFindNode = NULL;
    list_iterator_t it = {0u};
    list_iterator_init(&it, pToList);

    pFindNode = (list_node_t *)list_iterator_next(&it);

    while (pConditionFunc(&pLinker->node, pFindNode)) {
        pFindNode = (list_node_t *)list_iterator_next(&it);
    }

    if (pFindNode) {
        list_node_insertBefore(pToList, pFindNode, &pLinker->node);
    } else {
        list_node_push(pToList, &pLinker->node, LIST_TAIL);
    }
}

#ifdef __cplusplus
}
#endif
