/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 **/

#ifndef _LINKER_H_
#define _LINKER_H_

#include "typedef.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Define the list null value */
#define LIST_NULL                                                                                                                          \
    {                                                                                                                                      \
        NULL                                                                                                                               \
    }
#define ITERATION_NULL                                                                                                                     \
                                                                                                                                           \
    {                                                                                                                                      \
        NULL, NULL                                                                                                                         \
    }

#define LINKER_NULL                                                                                                                        \
    {                                                                                                                                      \
        {NULL}, NULL                                                                                                                       \
    }

/** @brief singly linked list node structure. */
typedef struct list_node {
    /* The pointer of the next node head. */
    struct list_node *pNext;
} list_node_t;

/** @brief singly linked list structure. */
typedef struct list {
    /* The pointer of the node head. */
    list_node_t *pHead;
} list_t;

/** @brief the direction of node operation structure. */
typedef enum {
    LIST_HEAD,
    LIST_TAIL
} list_direction_t;

typedef struct {
    /* The pointer of the current node. */
    list_node_t *pCurNode;

    /* The pointer of the current list. */
    list_t *pList;
} list_iterator_t;

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

    u8_t sts;

    /* The head string name, NULL is available */
    const char_t *pName;
} linker_head_t;

/** @brief The pointer of condition function in order to allow the application register a speicfic rules to mannage the list node */
typedef b_t (*pLinkerSpecificConditionFunc_t)(list_node_t *, list_node_t *);

b_t list_node_isExisted(list_t *pList, list_node_t *pNode);
u32_t list_size(list_t *pList);
b_t list_node_delete(list_t *pList, list_node_t *pTargetNode);
b_t list_node_insertBefore(list_t *pList, list_node_t *pBefore, list_node_t *pTargetNode);
b_t list_node_push(list_t *pList, list_node_t *pInNode, list_direction_t direction);
list_node_t *list_node_pop(list_t *pList, list_direction_t direction);
b_t list_iterator_init(list_iterator_t *pIterator, list_t *pList);
b_t list_iterator_next_condition(list_iterator_t *pIterator, list_node_t **ppOutNode);
list_node_t *list_iterator_next(list_iterator_t *pIterator);
void linker_list_transaction_common(linker_t *pLinker, list_t *pToList, list_direction_t direction);
void linker_list_transaction_specific(linker_t *pLinker, list_t *pToList, pLinkerSpecificConditionFunc_t pConditionFunc);
void os_memcpy(void *dst, const void *src, u32_t cnt);
void os_memset(void *dst, u8_t val, u32_t cnt);
i32_t os_memcmp(const void *dst, const void *src, u32_t cnt);
u32_t os_strlen(const uchar_t *str);

#ifdef __cplusplus
}
#endif

#endif /* _LINKER_H_ */
