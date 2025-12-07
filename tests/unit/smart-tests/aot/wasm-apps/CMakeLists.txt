# Copyright (C) 2019 Intel Corporation.  All rights reserved.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

cmake_minimum_required(VERSION 3.14)

# WAT to WASM conversion for enhanced AOT tests
find_program(WAT2WASM wat2wasm)

if(WAT2WASM)
    # Convert WAT files to WASM
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/simple_function.wasm
        COMMAND ${WAT2WASM} ${CMAKE_CURRENT_SOURCE_DIR}/simple_function.wat -o ${CMAKE_CURRENT_BINARY_DIR}/simple_function.wasm
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/simple_function.wat
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Converting simple_function.wat to WASM"
    )
    
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/multi_function.wasm
        COMMAND ${WAT2WASM} ${CMAKE_CURRENT_SOURCE_DIR}/multi_function.wat -o ${CMAKE_CURRENT_BINARY_DIR}/multi_function.wasm
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/multi_function.wat
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Converting multi_function.wat to WASM"
    )
    
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/memory_operations.wasm
        COMMAND ${WAT2WASM} ${CMAKE_CURRENT_SOURCE_DIR}/memory_operations.wat -o ${CMAKE_CURRENT_BINARY_DIR}/memory_operations.wasm
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/memory_operations.wat
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Converting memory_operations.wat to WASM"
    )
    
    # Create AOT files using wamrc
    find_program(WAMRC wamrc HINTS ${CMAKE_BINARY_DIR}/../../../wamr-compiler/build)
    
    if(WAMRC)
        add_custom_command(
            OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/simple_function.aot
            COMMAND ${WAMRC} -o ${CMAKE_CURRENT_BINARY_DIR}/simple_function.aot ${CMAKE_CURRENT_BINARY_DIR}/simple_function.wasm
            DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/simple_function.wasm
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Compiling simple_function.wasm to AOT"
        )
        
        add_custom_command(
            OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/multi_function.aot
            COMMAND ${WAMRC} -o ${CMAKE_CURRENT_BINARY_DIR}/multi_function.aot ${CMAKE_CURRENT_BINARY_DIR}/multi_function.wasm
            DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/multi_function.wasm
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Compiling multi_function.wasm to AOT"
        )
        
        add_custom_command(
            OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/memory_operations.aot
            COMMAND ${WAMRC} -o ${CMAKE_CURRENT_BINARY_DIR}/memory_operations.aot ${CMAKE_CURRENT_BINARY_DIR}/memory_operations.wasm
            DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/memory_operations.wasm
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Compiling memory_operations.wasm to AOT"
        )
        
        # Create target for all AOT files
        add_custom_target(aot_enhanced_wasm_files ALL
            DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/simple_function.aot ${CMAKE_CURRENT_BINARY_DIR}/multi_function.aot ${CMAKE_CURRENT_BINARY_DIR}/memory_operations.aot
        )
    else()
        message(WARNING "wamrc not found, AOT files will not be generated")
        # Create target for WASM files only
        add_custom_target(aot_enhanced_wasm_files ALL
            DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/simple_function.wasm ${CMAKE_CURRENT_BINARY_DIR}/multi_function.wasm ${CMAKE_CURRENT_BINARY_DIR}/memory_operations.wasm
        )
    endif()
else()
    message(WARNING "wat2wasm not found, WASM files will not be generated")
endif()