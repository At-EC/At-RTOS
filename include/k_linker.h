/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 **/
#ifndef _K_LINKER_H_
#define _K_LINKER_H_

#include "type_def.h"

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
struct list_node {
    /* The pointer of the next node head. */
    struct list_node *pNext;
};
typedef struct list_node list_node_t;

/** @brief singly linked list structure. */
struct list {
    /* The pointer of the node head. */
    struct list_node *pHead;
};
typedef struct list list_t;

/** @brief the direction of node operation structure. */
typedef enum {
    LIST_HEAD,
    LIST_TAIL
} list_direction_t;

typedef struct {
    /* The pointer of the current node. */
    struct list_node *pCurNode;

    /* The pointer of the current list. */
    struct list *pList;
} list_iterator_t;

/** @brief The linker structure help to mannage the singly-linked list. */
struct linker {
    /* The node */
    struct list_node node;

    /* The node in which list */
    struct list *pList;
};
typedef struct linker linker_t;

/** @brief The linker structure head is to mannage the rtos context. */
typedef struct {
    /* The linker is an important symbol to connect with same status node */
    struct linker linker;

    /* The head id */
    _u32_t id;

    _u8_t sts;

    /* The head string name, NULL is available */
    const _char_t *pName;
} linker_head_t;

/** @brief The pointer of condition function in order to allow the application register a speicfic rules to mannage the list node */
typedef _b_t (*pLinkerSpecificConditionFunc_t)(list_node_t *, list_node_t *);

_b_t list_node_isExisted(list_t *pList, list_node_t *pNode);
_u32_t list_size(list_t *pList);
void *list_head(list_t *pList);
_b_t list_node_delete(list_t *pList, list_node_t *pTargetNode);
_b_t list_node_insertBefore(list_t *pList, list_node_t *pBefore, list_node_t *pTargetNode);
_b_t list_node_push(list_t *pList, list_node_t *pInNode, list_direction_t direction);
list_node_t *list_node_pop(list_t *pList, list_direction_t direction);
_b_t list_iterator_init(list_iterator_t *pIterator, list_t *pList);
_b_t list_iterator_next_condition(list_iterator_t *pIterator, list_node_t **ppOutNode);
list_node_t *list_iterator_next(list_iterator_t *pIterator);
void linker_list_transaction_common(linker_t *pLinker, list_t *pToList, list_direction_t direction);
void linker_list_transaction_specific(linker_t *pLinker, list_t *pToList, pLinkerSpecificConditionFunc_t pConditionFunc);
void k_memcpy(void *dst, const void *src, _u32_t cnt);
void k_memset(void *dst, _u8_t val, _u32_t cnt);
_i32_t k_memcmp(const void *dst, const void *src, _u32_t cnt);
_u32_t k_strlen(const _uchar_t *str);

#endif /* _K_LINKER_H_ */
