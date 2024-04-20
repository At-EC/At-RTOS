/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 **/

#ifndef _LINKER_H_
#define _LINKER_H_

#include "list.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LINKER_NULL                                                                                                                        \
    {                                                                                                                                      \
        {NULL}, NULL                                                                                                                       \
    }

/** @brief The pointer of condition function in order to allow the application register a speicfic rules to mannage the list node */
typedef b_t (*pLinkerSpecificConditionFunc_t)(list_node_t *, list_node_t *);

/** @brief The linker structure help to mannage the singly-linked list. */
typedef struct {
    /* The node */
    list_node_t node;

    /* The node in which list */
    list_t *pList;
} linker_t;

/** @brief The linker structure head is to mannage the rtos context. */
typedef struct {
    /* The linker is an important symbol to connect with same status node */
    linker_t linker;

    /* The head id */
    u32_t id;

    /* The head string name, NULL is available */
    const char_t *pName;
} linker_head_t;

void linker_list_transaction_common(linker_t *pLinker, list_t *pToList, list_direction_t direction);
void linker_list_transaction_specific(linker_t *pLinker, list_t *pToList, pLinkerSpecificConditionFunc_t pConditionFunc);

#ifdef __cplusplus
}
#endif

#endif /* _LINKER_H_ */
