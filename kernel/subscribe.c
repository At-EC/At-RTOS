/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 **/
#include "kernel.h"
#include "postcode.h"
#include "trace.h"
#include "init.h"

/**
 * Local unique postcode.
 */
#define PC_EOR PC_IER(PC_OS_CMPT_PUBLISH_10)

/**
 * Data structure for location timer
 */
typedef struct {
    list_t callback_list;
} _sp_resource_t;

/**
 * Local timer resource
 */
_sp_resource_t g_sp_rsc = {0u};

/**
 * @brief Check if the publish unique id if is's invalid.
 *
 * @param id The provided unique id.
 *
 * @return The true is invalid, otherwise is valid.
 */
static b_t _publish_context_isInvalid(publish_context_t *pCurPub)
{
    u32_t start, end;
    INIT_SECTION_FIRST(INIT_SECTION_OS_PUBLISH_LIST, start);
    INIT_SECTION_LAST(INIT_SECTION_OS_PUBLISH_LIST, end);

    return ((u32_t)pCurPub < start || (u32_t)pCurPub >= end) ? true : false;
}

/**
 * @brief Check if the publish object if is's initialized.
 *
 * @param id The provided unique id.
 *
 * @return The true is initialized, otherwise is uninitialized.
 */
static b_t _publish_context_isInit(publish_context_t *pCurPub)
{
    return ((pCurPub) ? (((pCurPub->head.cs) ? (true) : (false))) : false);
}

/**
 * @brief Check if the subscribe unique id if is's invalid.
 *
 * @param id The provided unique id.
 *
 * @return The true is invalid, otherwise is valid.
 */
static b_t _subscribe_context_isInvalid(subscribe_context_t *pCurSub)
{
    u32_t start, end;
    INIT_SECTION_FIRST(INIT_SECTION_OS_SUBSCRIBE_LIST, start);
    INIT_SECTION_LAST(INIT_SECTION_OS_SUBSCRIBE_LIST, end);

    return ((u32_t)pCurSub < start || (u32_t)pCurSub >= end) ? true : false;
}

/**
 * @brief Check if the subscribe object if is's initialized.
 *
 * @param id The provided unique id.
 *
 * @return The true is initialized, otherwise is uninitialized.
 */
static b_t _subscribe_context_isInit(subscribe_context_t *pCurSub)
{
    return ((pCurSub) ? (((pCurSub->head.cs) ? (true) : (false))) : false);
}

/**
 * @brief Push one subscribe context into publish subscribe head list.
 *
 * @param pCurHead The pointer of the publish subscribe linker head.
 * @param pCurHead The pointer of the publish subscribe linker head.
 */
static void _subscribe_list_transfer_toTargetHead(linker_t *pLinker, publish_context_t *pCurPub)
{
    ENTER_CRITICAL_SECTION();

    list_t *pToPubList = (list_t *)&pCurPub->q_list;
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

    INIT_SECTION_FOREACH(INIT_SECTION_OS_PUBLISH_LIST, publish_context_t, pCurPublish)
    {
        if (_publish_context_isInvalid(pCurPublish)) {
            break;
        }

        if (_publish_context_isInit(pCurPublish)) {
            continue;
        }

        os_memset((char_t *)pCurPublish, 0x0u, sizeof(publish_context_t));
        pCurPublish->head.cs = CS_INITED;
        pCurPublish->head.pName = pName;

        EXIT_CRITICAL_SECTION();
        return (u32_t)pCurPublish;
    };

    EXIT_CRITICAL_SECTION();
    return 0u;
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

    publish_context_t *pCurSub = (publish_context_t *)pArgs[0].u32_val;
    const void *pPublishData = (const void *)pArgs[1].ptr_val;
    u16_t publishSize = (u16_t)pArgs[2].u16_val;
    b_t need = false;
    i32p_t postcode = 0;

    struct notify_callback *pNotify = NULL;
    list_iterator_t it = {0u};
    list_t *pList = (list_t *)&pCurSub->q_list;
    list_iterator_init(&it, pList);
    while (list_iterator_next_condition(&it, (void *)&pNotify)) {
        pNotify->updated++;
        os_memcpy((u8_t *)pNotify->pData, (const u8_t *)pPublishData, MINI_AB(publishSize, pNotify->len));
        if ((!pNotify->muted) && (pNotify->fn)) {
            need = true;
            pNotify->fn((void *)&pNotify->linker.node);
        }
    }

    if (need) {
        kernel_message_notification();
    }

    EXIT_CRITICAL_SECTION();
    return postcode;
}

void subscribe_notification(void *pNode)
{
    subscribe_context_t *pCurSubscribe = (subscribe_context_t *)CONTAINEROF(pNode, subscribe_context_t, notify);

    list_t *pCallback_list = (list_t *)&g_sp_rsc.callback_list;
    if (!list_node_isExisted(pCallback_list, &pCurSubscribe->call.node)) {
        list_node_push((list_t *)&g_sp_rsc.callback_list, &pCurSubscribe->call.node, LIST_HEAD);
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

    INIT_SECTION_FOREACH(INIT_SECTION_OS_SUBSCRIBE_LIST, subscribe_context_t, pCurSubscribe)
    {
        if (_subscribe_context_isInvalid(pCurSubscribe)) {
            break;
        }

        if (_subscribe_context_isInit(pCurSubscribe)) {
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
        pCurSubscribe->notify.fn = subscribe_notification;

        EXIT_CRITICAL_SECTION();
        return (u32_t)pCurSubscribe;
    };

    EXIT_CRITICAL_SECTION();
    return 0u;
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

    subscribe_context_t *pCurSub = (subscribe_context_t *)pArgs[0].u32_val;
    publish_context_t *pCurPub = (publish_context_t *)pArgs[1].u32_val;
    b_t isMute = (b_t)pArgs[2].b_val;
    pSubscribe_callbackFunc_t pCallFun = (pSubscribe_callbackFunc_t)(pArgs[3].ptr_val);

    pCurSub->pPublisher = pCurPub;

    pCurSub->notify.muted = isMute;
    pCurSub->call.pSubCallEntry = pCallFun;

    _subscribe_list_transfer_toTargetHead(&pCurSub->notify.linker, pCurPub);

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

    subscribe_context_t *pCurSub = (subscribe_context_t *)pArgs[0].u32_val;
    b_t ready = (pCurSub->accepted == pCurSub->notify.updated) ? (true) : (false);

    EXIT_CRITICAL_SECTION();
    return ready;
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

    subscribe_context_t *pCurSub = (subscribe_context_t *)pArgs[0].u32_val;
    u8_t *pDataBuffer = (u8_t *)pArgs[1].pv_val;
    u16_t *pDataLen = (u16_t *)pArgs[2].pv_val;

    if (pCurSub->accepted < pCurSub->notify.updated) {
        *pDataLen = MINI_AB(*pDataLen, pCurSub->notify.len);
        os_memcpy(pDataBuffer, pCurSub->notify.pData, *pDataLen);
        pCurSub->accepted = pCurSub->notify.updated;

        EXIT_CRITICAL_SECTION();
        return 0;
    }

    EXIT_CRITICAL_SECTION();
    return PC_OS_WAIT_UNAVAILABLE;
}

/**
 * @brief Initialize a new publish.
 *
 * @param pName The publish name.
 *
 * @return The publish unique id.
 */
u32_t _impl_publish_init(const char_t *pName)
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
 * @param size The data buffer size.
 * @param pName The subscribe name.
 *
 * @return Value The result fo subscribe init operation.
 */
u32_t _impl_subscribe_init(void *pDataAddr, u16_t size, const char_t *pName)
{
    if (!pDataAddr) {
        return OS_INVALID_ID_VAL;
    }

    if (!size) {
        return OS_INVALID_ID_VAL;
    }

    arguments_t arguments[] = {
        [0] = {.pv_val = (void *)pDataAddr},
        [1] = {.u16_val = (u16_t)size},
        [2] = {.pch_val = (const char_t *)pName},
    };

    return kernel_privilege_invoke((const void *)_subscribe_init_privilege_routine, arguments);
}

/**
 * @brief The subscribe register the corresponding publish.
 *
 * @param sub_ctx The subscribe unique id.
 * @param pub_ctx The publish unique id.
 * @param isMute The set of notification operation.
 * @param pFuncHandler The notification function handler pointer.
 *
 * @return Value The result fo subscribe init operation.
 */
i32p_t _impl_subscribe_register(u32_t sub_ctx, u32_t pub_ctx, b_t isMute, pSubscribe_callbackFunc_t pNotificationHandler)
{
    subscribe_context_t *pCtx_sub = (subscribe_context_t *)sub_ctx;
    publish_context_t *pCtx_pub = (publish_context_t *)pub_ctx;

    if (_subscribe_context_isInvalid(pCtx_sub)) {
        return PC_EOR;
    }

    if (!_subscribe_context_isInit(pCtx_sub)) {
        return PC_EOR;
    }

    if (_publish_context_isInvalid(pCtx_pub)) {
        return PC_EOR;
    }

    if (!_publish_context_isInit(pCtx_pub)) {
        return PC_EOR;
    }

    arguments_t arguments[] = {
        [0] = {.u32_val = (u32_t)sub_ctx},
        [1] = {.u32_val = (u32_t)pub_ctx},
        [2] = {.b_val = (b_t)isMute},
        [3] = {.ptr_val = (const void *)pNotificationHandler},
    };

    return kernel_privilege_invoke((const void *)_subscribe_register_privilege_routine, arguments);
}

/**
 * @brief The subscriber wait publisher put new data with a timeout option.
 *
 * @param sub_ctx The subscribe unique id.
 * @param pDataBuffer The pointer of data buffer.
 * @param pDataLen The pointer of data buffer len.
 *
 * @return Value The result of subscribe init operation.
 */
i32p_t _impl_subscribe_data_apply(u32_t sub_ctx, void *pDataBuffer, u16_t *pDataLen)
{
    subscribe_context_t *pCtx_sub = (subscribe_context_t *)sub_ctx;

    if (_subscribe_context_isInvalid(pCtx_sub)) {
        return PC_EOR;
    }

    if (!_subscribe_context_isInit(pCtx_sub)) {
        return PC_EOR;
    }

    if (!pDataBuffer) {
        return PC_EOR;
    }

    if (!pDataLen) {
        return PC_EOR;
    }

    arguments_t arguments[] = {
        [0] = {.u32_val = (u32_t)sub_ctx},
        [1] = {.pv_val = (void *)pDataBuffer},
        [2] = {.pv_val = (void *)pDataLen},
    };

    return kernel_privilege_invoke((const void *)_subscribe_apply_data_privilege_routine, arguments);
}

/**
 * @brief To check if the publisher submits new data and that is not applied by subscriber.
 *
 * @param sub_ctx The subscribe unique id.
 *
 * @return Value The result of subscribe data is ready.
 */
b_t _impl_subscribe_data_is_ready(u32_t sub_ctx)
{
    subscribe_context_t *pCtx_sub = (subscribe_context_t *)sub_ctx;

    if (_subscribe_context_isInvalid(pCtx_sub)) {
        return PC_EOR;
    }

    if (!_subscribe_context_isInit(pCtx_sub)) {
        return PC_EOR;
    }

    arguments_t arguments[] = {
        [0] = {.u32_val = (u32_t)sub_ctx},
    };

    return kernel_privilege_invoke((const void *)_subscribe_data_is_ready_privilege_routine, arguments);
}

/**
 * @brief Publisher Submits the report data.
 *
 * @param pub_ctx The publish unique id.
 * @param pDataAddr The pointer of the data buffer address.
 * @param dataSize The data buffer size.
 *
 * @return Value The result of the publisher data operation.
 */
i32p_t _impl_publish_data_submit(u32_t pub_ctx, const void *pPublishData, u16_t publishSize)
{
    publish_context_t *pCtx_sub = (publish_context_t *)pub_ctx;

    if (_publish_context_isInvalid(pCtx_sub)) {
        return PC_EOR;
    }

    if (!_publish_context_isInit(pCtx_sub)) {
        return PC_EOR;
    }

    arguments_t arguments[] = {
        [0] = {.u32_val = (u32_t)pub_ctx},
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
    list_t *pListPending = (list_t *)&g_sp_rsc.callback_list;

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
