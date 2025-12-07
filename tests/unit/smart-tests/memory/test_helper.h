/*
 * Copyright (C) 2019 Intel Corporation. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#ifndef _TEST_HELPER_H_
#define _TEST_HELPER_H_

#include "wasm_runtime.h"
#include <gtest/gtest.h>

// RunningMode enum is already defined in wasm_export.h:
// Mode_Interp = 1, Mode_Fast_JIT, Mode_LLVM_JIT, Mode_Multi_Tier_JIT
// We can use Mode_Interp and Mode_LLVM_JIT directly

/**
 * @brief RAII wrapper for WAMR runtime initialization and cleanup
 *
 * Template parameter for heap size configuration (default 512KB)
 */
template<uint32_t HEAP_SIZE = 512 * 1024>
class WAMRRuntimeRAII {
public:
    WAMRRuntimeRAII() {
        RuntimeInitArgs init_args;
        memset(&init_args, 0, sizeof(RuntimeInitArgs));
        init_args.mem_alloc_type = Alloc_With_System_Allocator;

        if (!wasm_runtime_full_init(&init_args)) {
            throw std::runtime_error("Failed to initialize WAMR runtime");
        }
    }

    ~WAMRRuntimeRAII() {
        wasm_runtime_destroy();
    }

    // Non-copyable and non-movable
    WAMRRuntimeRAII(const WAMRRuntimeRAII&) = delete;
    WAMRRuntimeRAII& operator=(const WAMRRuntimeRAII&) = delete;
    WAMRRuntimeRAII(WAMRRuntimeRAII&&) = delete;
    WAMRRuntimeRAII& operator=(WAMRRuntimeRAII&&) = delete;

    static constexpr uint32_t heap_size = HEAP_SIZE;
};

#endif /* end of _TEST_HELPER_H_ */