/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 **/

#include "kernel.h"
#include "postcode.h"
#include "trace.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Local unique postcode.
 */
#define _PC_CMPT_FAILED PC_FAILED(PC_CMPT_PUBLISH_9)

/**
 * @brief Get the publish context based on provided unique id.
 *
 * @param id The publish unique id.
 *
 * @return The pointer of the current unique id publish context.
 */
static publish_context_t *_publish_object_contextGet(os_id_t id)
{
    return (publish_context_t *)(kernel_member_unified_id_toContainerAddress(id));
}

/**
 * @brief Get the publish init list head.
 *
 * @return The value of the init list head.
 */
static list_t *_publish_list_initHeadGet(void)
{
    return (list_t *)kernel_member_list_get(KERNEL_MEMBER_PUBLISH, KERNEL_MEMBER_LIST_PUBLISH_INIT);
}

/**
 * @brief Get the pending publish list head.
 *
 * @return The value of the pending list head.
 */
static list_t *_publish_list_pendingHeadGet(void)
{
    return (list_t *)kernel_member_list_get(KERNEL_MEMBER_PUBLISH, KERNEL_MEMBER_LIST_PUBLISH_PEND);
}

/**
 * @brief Pick up a highest priority thread that blocking by the event pending list.
 *
 * @param The event unique id.
 *
 * @return The highest blocking thread head.
 */
static list_t *_publish_list_subscribeHeadGet(os_id_t id)
{
    publish_context_t *pCurPublish = _publish_object_contextGet(id);

    return (list_t *)((pCurPublish) ? (&pCurPublish->subscribeListHead) : (NULL));
}

/**
 * @brief Push one publish context into init list.
 *
 * @param pCurHead The pointer of the publish linker head.
 */
static void _publish_list_transfer_toInit(linker_head_t *pCurHead)
{
    ENTER_CRITICAL_SECTION();

    list_t *pToInitList = (list_t *)_publish_list_initHeadGet();
    linker_list_transaction_common(&pCurHead->linker, pToInitList, LIST_TAIL);

    EXIT_CRITICAL_SECTION();
}

/**
 * @brief Check if the publish unique id if is's invalid.
 *
 * @param id The provided unique id.
 *
 * @return The true is invalid, otherwise is valid.
 */
static b_t _publish_id_isInvalid(u32_t id)
{
    return kernel_member_unified_id_isInvalid(KERNEL_MEMBER_PUBLISH, id);
}

/**
 * @brief Check if the publish object if is's initialized.
 *
 * @param id The provided unique id.
 *
 * @return The true is initialized, otherwise is uninitialized.
 */
static b_t _publish_object_isInit(u32_t id)
{
    publish_context_t *pCurPublish = _publish_object_contextGet(id);

    return ((pCurPublish) ? (((pCurPublish->head.linker.pList) ? (TRUE) : (FALSE))) : FALSE);
}

/**
 * @brief Get the subscribe context based on provided unique id.
 *
 * @param id The subscribe unique id.
 *
 * @return The pointer of the current unique id subscribe context.
 */
static subscribe_context_t *_subscribe_object_contextGet(os_id_t id)
{
    return (subscribe_context_t *)(kernel_member_unified_id_toContainerAddress(id));
}

/**
 * @brief Get the subscribe init list head.
 *
 * @return The value of the init list head.
 */
static list_t *_subscribe_list_initHeadGet(void)
{
    return (list_t *)kernel_member_list_get(KERNEL_MEMBER_SUBSCRIBE, KERNEL_MEMBER_LIST_SUBSCRIBE_INIT);
}

/**
 * @brief Push one subscribe context into init list.
 *
 * @param pCurHead The pointer of the subscribe linker head.
 */
static void _subscribe_list_transfer_toInit(linker_head_t *pCurHead)
{
    ENTER_CRITICAL_SECTION();

    list_t *pToInitList = (list_t *)_subscribe_list_initHeadGet();
    linker_list_transaction_common(&pCurHead->linker, pToInitList, LIST_TAIL);

    EXIT_CRITICAL_SECTION();
}

/**
 * @brief Push one subscribe context into publish subscribe head list.
 *
 * @param pCurHead The pointer of the publish subscribe linker head.
 * @param pCurHead The pointer of the publish subscribe linker head.
 */
static void _subscribe_list_transfer_toTargetHead(linker_head_t *pCurHead, os_id_t pub)
{
    ENTER_CRITICAL_SECTION();

    list_t *pToPubList = (list_t *)_publish_list_subscribeHeadGet(pub);
    linker_list_transaction_common(&pCurHead->linker, pToPubList, LIST_TAIL);

    EXIT_CRITICAL_SECTION();
}

/**
 * @brief Check if the subscribe unique id if is's invalid.
 *
 * @param id The provided unique id.
 *
 * @return The true is invalid, otherwise is valid.
 */
static b_t _subscribe_id_isInvalid(u32_t id)
{
    return kernel_member_unified_id_isInvalid(KERNEL_MEMBER_SUBSCRIBE, id);
}

/**
 * @brief Check if the subscribe object if is's initialized.
 *
 * @param id The provided unique id.
 *
 * @return The true is initialized, otherwise is uninitialized.
 */
static b_t _subscribe_object_isInit(u32_t id)
{
    subscribe_context_t *pCurSubscribe = _subscribe_object_contextGet(id);

    return ((pCurSubscribe) ? (((pCurSubscribe->head.linker.pList) ? (TRUE) : (FALSE))) : FALSE);
}

/**
 * @brief It's sub-routine running at privilege mode.
 *
 * @param pArgs The function argument packages.
 *
 * @return The result of privilege routine.
 */
static u32_t _publish_init_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();

    const char_t *pName = (const char_t *)(pArgs[0].pch_val);
    u32_t endAddr = 0u;
    publish_context_t *pCurPublish = NULL;

    pCurPublish = (publish_context_t *)kernel_member_id_toContainerStartAddress(KERNEL_MEMBER_PUBLISH);
    endAddr = (u32_t)kernel_member_id_toContainerEndAddress(KERNEL_MEMBER_PUBLISH);
    do {
        os_id_t id = kernel_member_containerAddress_toUnifiedid((u32_t)pCurPublish);
        if (_publish_id_isInvalid(id)) {
            break;
        }

        if (_publish_object_isInit(id)) {
            continue;
        }

        os_memset((char_t *)pCurPublish, 0x0u, sizeof(publish_context_t));
        pCurPublish->head.id = id;
        pCurPublish->head.pName = pName;
        pCurPublish->refresh_count = 0u;

        _publish_list_transfer_toInit((linker_head_t *)&pCurPublish->head);

        EXIT_CRITICAL_SECTION();
        return id;

    } while ((u32_t)++pCurPublish < endAddr);

    EXIT_CRITICAL_SECTION();
    return OS_INVALID_ID;
}

/**
 * @brief It's sub-routine running at privilege mode.
 *
 * @param pArgs The function argument packages.
 *
 * @return The result of privilege routine.
 */
static u32_t _publish_data_submit_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();

    os_id_t id = (os_id_t)pArgs[0].u32_val;
    const void *pPublishData = (const void *)pArgs[1].ptr_val;
    u16_t publishSize = (u16_t)pArgs[2].u16_val;
    b_t request = FALSE;
    u32p_t postcode = PC_SC_SUCCESS;
    publish_context_t *pCurPublish = _publish_object_contextGet(id);
    pCurPublish->refresh_count++;

    list_iterator_t it = {0u};
    list_iterator_init(&it, _publish_list_subscribeHeadGet(id));
    subscribe_context_t *pCurSubscribe = NULL;
    while (list_iterator_next_condition(&it, (void *)&pCurSubscribe)) {
        os_memcpy((u8_t *)pCurSubscribe->callEntry.pDataAddress, (const u8_t *)pPublishData,
                  MINI(publishSize, pCurSubscribe->callEntry.dataSize));

        if ((!pCurSubscribe->isMute) && (pCurSubscribe->callEntry.pNotificationHandler)) {
            list_t *pListPending = (list_t *)_publish_list_pendingHeadGet();
            if (!list_node_isExisted(pListPending, &pCurSubscribe->callEntry.node)) {
                list_node_push(_publish_list_pendingHeadGet(), &pCurSubscribe->callEntry.node, LIST_HEAD);
                request = TRUE;
            }
        }
    }

    if (request) {
        kernel_message_notification();
    }

    EXIT_CRITICAL_SECTION();
    return postcode;
}

/**
 * @brief It's sub-routine running at privilege mode.
 *
 * @param pArgs The function argument packages.
 *
 * @return The result of privilege routine.
 */
static u32_t _subscribe_init_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();

    void *pData = (void *)pArgs[0].pv_val;
    u16_t size = (u16_t)pArgs[1].u16_val;
    const char_t *pName = (const char_t *)(pArgs[2].pch_val);
    u32_t endAddr = 0u;
    subscribe_context_t *pCurSubscribe = NULL;

    pCurSubscribe = (subscribe_context_t *)kernel_member_id_toContainerStartAddress(KERNEL_MEMBER_SUBSCRIBE);
    endAddr = (u32_t)kernel_member_id_toContainerEndAddress(KERNEL_MEMBER_SUBSCRIBE);
    do {
        os_id_t id = kernel_member_containerAddress_toUnifiedid((u32_t)pCurSubscribe);
        if (_subscribe_id_isInvalid(id)) {
            break;
        }

        if (_subscribe_object_isInit(id)) {
            continue;
        }

        os_memset((char_t *)pCurSubscribe, 0x0u, sizeof(subscribe_context_t));
        pCurSubscribe->head.id = id;
        pCurSubscribe->head.pName = pName;
        pCurSubscribe->hold = OS_INVALID_ID;
        pCurSubscribe->last_count = 0u;
        pCurSubscribe->isMute = FALSE;
        pCurSubscribe->callEntry.pDataAddress = pData;
        pCurSubscribe->callEntry.dataSize = size;
        pCurSubscribe->callEntry.pNotificationHandler = NULL;

        _subscribe_list_transfer_toInit((linker_head_t *)&pCurSubscribe->head);

        EXIT_CRITICAL_SECTION();
        return id;

    } while ((u32_t)++pCurSubscribe < endAddr);

    EXIT_CRITICAL_SECTION();
    return OS_INVALID_ID;
}

/**
 * @brief It's sub-routine running at privilege mode.
 *
 * @param pArgs The function argument packages.
 *
 * @return The result of privilege routine.
 */
static u32_t _subscribe_register_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();

    os_id_t sub = (os_id_t)pArgs[0].u32_val;
    os_id_t pub = (os_id_t)pArgs[1].u32_val;
    b_t isMute = (b_t)pArgs[2].b_val;
    pSubscribe_callbackFunc_t pCallFun = (pSubscribe_callbackFunc_t)(pArgs[3].ptr_val);

    subscribe_context_t *pCurSubscribe = _subscribe_object_contextGet(sub);
    pCurSubscribe->hold = pub;
    pCurSubscribe->isMute = isMute;
    pCurSubscribe->callEntry.pNotificationHandler = pCallFun;

    _subscribe_list_transfer_toTargetHead((linker_head_t *)&pCurSubscribe->head, pub);

    EXIT_CRITICAL_SECTION();
    return PC_SC_SUCCESS;
}

/**
 * @brief It's sub-routine running at privilege mode.
 *
 * @param pArgs The function argument packages.
 *
 * @return The result of privilege routine.
 */
static u32_t _subscribe_data_is_ready_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();

    u32_t id = (u32_t)pArgs[0].u32_val;

    subscribe_context_t *pCurSubscribe = _subscribe_object_contextGet(id);
    publish_context_t *pCurPublish = _publish_object_contextGet(pCurSubscribe->hold);

    EXIT_CRITICAL_SECTION();
    return (pCurSubscribe->last_count == pCurPublish->refresh_count) ? (TRUE) : (FALSE);
}

/**
 * @brief It's sub-routine running at privilege mode.
 *
 * @param pArgs The function argument packages.
 *
 * @return The result of privilege routine.
 */
static u32_t _subscribe_apply_data_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();

    u32_t id = (u32_t)pArgs[0].u32_val;
    u8_t *pDataBuffer = (u8_t *)pArgs[1].pv_val;
    u16_t *pDataLen = (u16_t *)pArgs[2].pv_val;

    subscribe_context_t *pCurSubscribe = _subscribe_object_contextGet(id);
    publish_context_t *pCurPublish = _publish_object_contextGet(pCurSubscribe->hold);

    if (pCurSubscribe->last_count < pCurPublish->refresh_count) {
        *pDataLen = MINI(*pDataLen, pCurSubscribe->callEntry.dataSize);
        os_memcpy(pDataBuffer, pCurSubscribe->callEntry.pDataAddress, *pDataLen);
        pCurSubscribe->last_count = pCurPublish->refresh_count;

        EXIT_CRITICAL_SECTION();
        return PC_SC_SUCCESS;
    }

    EXIT_CRITICAL_SECTION();
    return PC_SC_UNAVAILABLE;
}

/**
 * @brief Convert the internal os id to kernel member number.
 *
 * @param id The provided unique id.
 *
 * @return The value of member number.
 */
u32_t _impl_publish_os_id_to_number(os_id_t id)
{
    if (_publish_id_isInvalid(id)) {
        return 0u;
    }

    return (u32_t)((id - kernel_member_id_toUnifiedIdStart(KERNEL_MEMBER_PUBLISH)) / sizeof(publish_context_t));
}

/**
 * @brief Convert the internal os id to kernel member number.
 *
 * @param id The provided unique id.
 *
 * @return The value of member number.
 */
u32_t _impl_subscribe_os_id_to_number(os_id_t id)
{
    if (_subscribe_id_isInvalid(id)) {
        return 0u;
    }

    return (u32_t)((id - kernel_member_id_toUnifiedIdStart(KERNEL_MEMBER_SUBSCRIBE)) / sizeof(subscribe_context_t));
}

/**
 * @brief Initialize a new publish.
 *
 * @param pName The publish name.
 *
 * @return The publish unique id.
 */
os_id_t _impl_publish_init(const char_t *pName)
{
    arguments_t arguments[] = {
        [0] = {.pch_val = (const char_t *)pName},
    };

    return kernel_privilege_invoke((const void *)_publish_init_privilege_routine, arguments);
}

/**
 * @brief Initialize a new subscribe.
 *
 * @param pDataAddr The pointer of the data buffer address.
 * @param dataSize The data buffer size.
 * @param pName The subscribe name.
 *
 * @return Value The result fo subscribe init operation.
 */
os_id_t _impl_subscribe_init(void *pDataAddr, u16_t dataSize, const char_t *pName)
{
    if (!pDataAddr) {
        return _PC_CMPT_FAILED;
    }

    if (!dataSize) {
        return _PC_CMPT_FAILED;
    }

    arguments_t arguments[] = {
        [0] = {.pv_val = (void *)pDataAddr},
        [1] = {.u16_val = (u16_t)dataSize},
        [2] = {.pch_val = (const char_t *)pName},
    };

    return kernel_privilege_invoke((const void *)_subscribe_init_privilege_routine, arguments);
}

/**
 * @brief The subscribe register the corresponding publish.
 *
 * @param subscribe_id The subscribe unique id.
 * @param publish_id The publish unique id.
 * @param isMute The set of notification operation.
 * @param pFuncHandler The notification function handler pointer.
 *
 * @return Value The result fo subscribe init operation.
 */
u32p_t _impl_subscribe_register(os_id_t subscribe_id, os_id_t publish_id, b_t isMute, pSubscribe_callbackFunc_t pNotificationHandler)
{
    if (_subscribe_id_isInvalid(subscribe_id)) {
        return _PC_CMPT_FAILED;
    }

    if (!_subscribe_object_isInit(subscribe_id)) {
        return _PC_CMPT_FAILED;
    }

    if (_publish_id_isInvalid(publish_id)) {
        return _PC_CMPT_FAILED;
    }

    if (!_publish_object_isInit(publish_id)) {
        return _PC_CMPT_FAILED;
    }

    arguments_t arguments[] = {
        [0] = {.u32_val = (u32_t)subscribe_id},
        [1] = {.u32_val = (u32_t)publish_id},
        [2] = {.b_val = (b_t)isMute},
        [3] = {.ptr_val = (const void *)pNotificationHandler},
    };

    return kernel_privilege_invoke((const void *)_subscribe_register_privilege_routine, arguments);
}

/**
 * @brief The subscriber wait publisher put new data with a timeout option.
 *
 * @param subscribe_id The subscribe unique id.
 * @param pDataBuffer The pointer of data buffer.
 * @param pDataLen The pointer of data buffer len.
 *
 * @return Value The result of subscribe init operation.
 */
u32p_t _impl_subscribe_data_apply(os_id_t subscribe_id, void *pDataBuffer, u16_t *pDataLen)
{
    if (_subscribe_id_isInvalid(subscribe_id)) {
        return _PC_CMPT_FAILED;
    }

    if (!_subscribe_object_isInit(subscribe_id)) {
        return _PC_CMPT_FAILED;
    }

    if (!pDataBuffer) {
        return _PC_CMPT_FAILED;
    }

    if (!pDataLen) {
        return _PC_CMPT_FAILED;
    }

    arguments_t arguments[] = {
        [0] = {.u32_val = (u32_t)subscribe_id},
        [1] = {.pv_val = (void *)pDataBuffer},
        [2] = {.pv_val = (void *)pDataLen},
    };

    return kernel_privilege_invoke((const void *)_subscribe_apply_data_privilege_routine, arguments);
}

/**
 * @brief To check if the publisher submits new data and that is not applied by subscriber.
 *
 * @param subscribe_id The subscribe unique id.
 *
 * @return Value The result of subscribe data is ready.
 */
b_t _impl_subscribe_data_is_ready(os_id_t subscribe_id)
{
    if (_subscribe_id_isInvalid(subscribe_id)) {
        return _PC_CMPT_FAILED;
    }

    if (!_subscribe_object_isInit(subscribe_id)) {
        return _PC_CMPT_FAILED;
    }

    arguments_t arguments[] = {
        [0] = {.u32_val = (u32_t)subscribe_id},
    };

    return kernel_privilege_invoke((const void *)_subscribe_data_is_ready_privilege_routine, arguments);
}

/**
 * @brief Publisher Submits the report data.
 *
 * @param id The publish unique id.
 * @param pDataAddr The pointer of the data buffer address.
 * @param dataSize The data buffer size.
 *
 * @return Value The result of the publisher data operation.
 */
u32p_t _impl_publish_data_submit(os_id_t id, const void *pPublishData, u16_t publishSize)
{
    if (_publish_id_isInvalid(id)) {
        return _PC_CMPT_FAILED;
    }

    if (!_publish_object_isInit(id)) {
        return _PC_CMPT_FAILED;
    }

    arguments_t arguments[] = {
        [0] = {.u32_val = (u32_t)id},
        [1] = {.ptr_val = (const void *)pPublishData},
        [2] = {.u16_val = (u16_t)publishSize},
    };

    return kernel_privilege_invoke((const void *)_publish_data_submit_privilege_routine, arguments);
}

/**
 * @brief Subscribe callback function handle in the kernel thread.
 */
void _impl_publish_pending_handler(void)
{
    list_t *pListPending = (list_t *)_publish_list_pendingHeadGet();

    ENTER_CRITICAL_SECTION();
    struct callSubEntry *pCallFuncEntry = (struct callSubEntry *)list_node_pop(pListPending, LIST_TAIL);
    EXIT_CRITICAL_SECTION();

    while (pCallFuncEntry) {
        if (pCallFuncEntry->pNotificationHandler) {
            pCallFuncEntry->pNotificationHandler(pCallFuncEntry->pDataAddress, pCallFuncEntry->dataSize);
        }

        ENTER_CRITICAL_SECTION();
        pCallFuncEntry = (struct callSubEntry *)list_node_pop(pListPending, LIST_TAIL);
        EXIT_CRITICAL_SECTION();
    }
}

/**
 * @brief Get publish snapshot informations.
 *
 * @param instance The event instance number.
 * @param pMsgs The kernel snapshot information pointer.
 *
 * @return TRUE: Operation pass, FALSE: Operation failed.
 */
b_t publish_snapshot(u32_t instance, kernel_snapshot_t *pMsgs)
{
#if defined KTRACE
    publish_context_t *pCurPublish = NULL;
    u32_t offset = 0u;
    os_id_t id = OS_INVALID_ID;

    ENTER_CRITICAL_SECTION();

    offset = sizeof(publish_context_t) * instance;
    pCurPublish = (publish_context_t *)(kernel_member_id_toContainerStartAddress(KERNEL_MEMBER_PUBLISH) + offset);
    id = kernel_member_containerAddress_toUnifiedid((u32_t)pCurPublish);
    os_memset((u8_t *)pMsgs, 0x0u, sizeof(kernel_snapshot_t));

    if (_publish_id_isInvalid(id)) {
        EXIT_CRITICAL_SECTION();
        return FALSE;
    }

    if (pCurPublish->head.linker.pList == _publish_list_initHeadGet()) {
        pMsgs->pState = "init";
    } else if (pCurPublish->head.linker.pList) {
        pMsgs->pState = "*";
    } else {
        pMsgs->pState = "unused";

        EXIT_CRITICAL_SECTION();
        return FALSE;
    }

    pMsgs->id = pCurPublish->head.id;
    pMsgs->pName = pCurPublish->head.pName;

    pMsgs->publish.refresh_count = pCurPublish->refresh_count;

    EXIT_CRITICAL_SECTION();
    return TRUE;
#else
    return FALSE;
#endif
}

#ifdef __cplusplus
}
#endif
