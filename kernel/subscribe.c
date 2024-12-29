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
#define _PCER PC_IER(PC_OS_CMPT_PUBLISH_10)

/**
 * Data structure for location timer
 */
typedef struct {
    list_t callback_list;
} _sp_resource_t;

/**
 * Local timer resource
 */
_sp_resource_t g_sp_resource = {0u};

/**
 * @brief Get the publish context based on provided unique id.
 *
 * @param id The publish unique id.
 *
 * @return The pointer of the current unique id publish context.
 */
static publish_context_t *_publish_context_get(os_id_t id)
{
    return (publish_context_t *)(kernel_member_unified_id_toContainerAddress(id));
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
static b_t _publish_id_isInit(u32_t id)
{
    publish_context_t *pCurPublish = _publish_context_get(id);

    return ((pCurPublish) ? (((pCurPublish->head.cs) ? (TRUE) : (FALSE))) : FALSE);
}

/**
 * @brief Get the subscribe context based on provided unique id.
 *
 * @param id The subscribe unique id.
 *
 * @return The pointer of the current unique id subscribe context.
 */
static subscribe_context_t *_subscribe_context_get(os_id_t id)
{
    return (subscribe_context_t *)(kernel_member_unified_id_toContainerAddress(id));
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
static b_t _subscribe_id_isInit(u32_t id)
{
    subscribe_context_t *pCurSubscribe = _subscribe_context_get(id);

    return ((pCurSubscribe) ? (((pCurSubscribe->head.cs) ? (TRUE) : (FALSE))) : FALSE);
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
    publish_context_t *pCurPublish = _publish_context_get(id);

    return (list_t *)((pCurPublish) ? (&pCurPublish->subscribeListHead) : (NULL));
}

/**
 * @brief Push one subscribe context into publish subscribe head list.
 *
 * @param pCurHead The pointer of the publish subscribe linker head.
 * @param pCurHead The pointer of the publish subscribe linker head.
 */
static void _subscribe_list_transfer_toTargetHead(linker_t *pLinker, os_id_t pub)
{
    ENTER_CRITICAL_SECTION();

    list_t *pToPubList = (list_t *)_publish_list_subscribeHeadGet(pub);
    linker_list_transaction_common(pLinker, pToPubList, LIST_TAIL);

    EXIT_CRITICAL_SECTION();
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

        if (_publish_id_isInit(id)) {
            continue;
        }

        os_memset((char_t *)pCurPublish, 0x0u, sizeof(publish_context_t));
        pCurPublish->head.cs = CS_INITED;
        pCurPublish->head.pName = pName;

        EXIT_CRITICAL_SECTION();
        return id;

    } while ((u32_t)++pCurPublish < endAddr);

    EXIT_CRITICAL_SECTION();
    return OS_INVALID_ID_VAL;
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
    b_t need = FALSE;
    i32p_t postcode = 0;
    publish_context_t *pCurPublish = _publish_context_get(id);

    struct notify_callback *pNotify = NULL;
    list_iterator_t it = {0u};
    list_iterator_init(&it, _publish_list_subscribeHeadGet(id));
    while (list_iterator_next_condition(&it, (void *)&pNotify)) {
        pNotify->updated++;
        os_memcpy((u8_t *)pNotify->pData, (const u8_t *)pPublishData, MINI_AB(publishSize, pNotify->len));
        if ((!pNotify->muted) && (pNotify->fn)) {
            need = true;
            pNotify->fn(pNotify);
        }
    }

    if (need) {
        kernel_message_notification();
    }

    EXIT_CRITICAL_SECTION();
    return postcode;
}

static void _subscribe_notification(void *pLinker)
{
    subscribe_context_t *pCurSubscribe = (subscribe_context_t *)CONTAINEROF(pLinker, subscribe_context_t, notify);

    list_t *pCallback_list = (list_t *)&g_sp_resource.callback_list;
    if (!list_node_isExisted(pCallback_list, &pCurSubscribe->call.node)) {
        list_node_push((list_t *)&g_sp_resource.callback_list, &pCurSubscribe->call.node, LIST_HEAD);
    }
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

        if (_subscribe_id_isInit(id)) {
            continue;
        }

        os_memset((char_t *)pCurSubscribe, 0x0u, sizeof(subscribe_context_t));
        pCurSubscribe->head.cs = CS_INITED;
        pCurSubscribe->head.pName = pName;
        pCurSubscribe->pPublisher = NULL;
        pCurSubscribe->accepted = 0u;

        pCurSubscribe->notify.pData = pData;
        pCurSubscribe->notify.len = size;
        pCurSubscribe->notify.muted = false;
        pCurSubscribe->notify.fn = _subscribe_notification;

        EXIT_CRITICAL_SECTION();
        return id;

    } while ((u32_t)++pCurSubscribe < endAddr);

    EXIT_CRITICAL_SECTION();
    return OS_INVALID_ID_VAL;
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

    subscribe_context_t *pCurSubscribe = _subscribe_context_get(sub);
    pCurSubscribe->pPublisher = _publish_context_get(pub);

    pCurSubscribe->notify.muted = isMute;
    pCurSubscribe->call.pSubCallEntry = pCallFun;

    _subscribe_list_transfer_toTargetHead(&pCurSubscribe->notify.linker, pub);

    EXIT_CRITICAL_SECTION();
    return 0;
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

    subscribe_context_t *pCurSubscribe = _subscribe_context_get(id);

    EXIT_CRITICAL_SECTION();
    return (pCurSubscribe->accepted == pCurSubscribe->notify.updated) ? (true) : (false);
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

    subscribe_context_t *pCurSubscribe = _subscribe_context_get(id);

    if (pCurSubscribe->accepted < pCurSubscribe->notify.updated) {
        *pDataLen = MINI_AB(*pDataLen, pCurSubscribe->notify.len);
        os_memcpy(pDataBuffer, pCurSubscribe->notify.pData, *pDataLen);
        pCurSubscribe->accepted = pCurSubscribe->notify.updated;

        EXIT_CRITICAL_SECTION();
        return 0;
    }

    EXIT_CRITICAL_SECTION();
    return PC_OS_WAIT_UNAVAILABLE;
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
        return OS_INVALID_ID_VAL;
    }

    if (!dataSize) {
        return OS_INVALID_ID_VAL;
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
i32p_t _impl_subscribe_register(os_id_t subscribe_id, os_id_t publish_id, b_t isMute, pSubscribe_callbackFunc_t pNotificationHandler)
{
    if (_subscribe_id_isInvalid(subscribe_id)) {
        return _PCER;
    }

    if (!_subscribe_id_isInit(subscribe_id)) {
        return _PCER;
    }

    if (_publish_id_isInvalid(publish_id)) {
        return _PCER;
    }

    if (!_publish_id_isInit(publish_id)) {
        return _PCER;
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
i32p_t _impl_subscribe_data_apply(os_id_t subscribe_id, void *pDataBuffer, u16_t *pDataLen)
{
    if (_subscribe_id_isInvalid(subscribe_id)) {
        return _PCER;
    }

    if (!_subscribe_id_isInit(subscribe_id)) {
        return _PCER;
    }

    if (!pDataBuffer) {
        return _PCER;
    }

    if (!pDataLen) {
        return _PCER;
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
        return _PCER;
    }

    if (!_subscribe_id_isInit(subscribe_id)) {
        return _PCER;
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
i32p_t _impl_publish_data_submit(os_id_t id, const void *pPublishData, u16_t publishSize)
{
    if (_publish_id_isInvalid(id)) {
        return _PCER;
    }

    if (!_publish_id_isInit(id)) {
        return _PCER;
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
    list_t *pListPending = (list_t *)&g_sp_resource.callback_list;

    ENTER_CRITICAL_SECTION();
    struct subscribe_callback *pCallFuncEntry = (struct subscribe_callback *)list_node_pop(pListPending, LIST_TAIL);
    EXIT_CRITICAL_SECTION();

    while (pCallFuncEntry) {
        if (pCallFuncEntry->pSubCallEntry) {
            subscribe_context_t *pCurSubscribe = (subscribe_context_t *)CONTAINEROF(pCallFuncEntry, subscribe_context_t, call);
            pCallFuncEntry->pSubCallEntry(pCurSubscribe->notify.pData, pCurSubscribe->notify.len);
        }

        ENTER_CRITICAL_SECTION();
        pCallFuncEntry = (struct subscribe_callback *)list_node_pop(pListPending, LIST_TAIL);
        EXIT_CRITICAL_SECTION();
    }
}

#ifdef __cplusplus
}
#endif
