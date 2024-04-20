/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 **/

#include "trace.h"
#include "postcode.h"
#include "configuration.h"
#include "linker.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Local trace postcode contrainer
 */
vu32_t g_postcode_cmpt_failed_container[POSTCODE_COMPONENT_NUMBER] = {0u};

/**
 * @brief To trace the last failed postcode.
 *
 * @param postcode The postcode number.
 *
 * @return Return the original argument postcode.
 */
u32p_t _impl_postcode_cmpt_failed_save(u32p_t postcode)
{
    u32_t cmpt_instance = PC_TO_CMPT_ID(postcode);

    if (cmpt_instance >= POSTCODE_COMPONENT_NUMBER) {
        return postcode;
    }

    if (FLAG(postcode & PC_FAILED_CODE_MASK)) {
        g_postcode_cmpt_failed_container[cmpt_instance] = (postcode & PC_LINE_NUMBER_MASK);

        _impl_trace_postcode_snapshot_print();
        /* Dead loop */
        while (1) {};
    }

    return postcode;
}

#ifdef __cplusplus
}
#endif
