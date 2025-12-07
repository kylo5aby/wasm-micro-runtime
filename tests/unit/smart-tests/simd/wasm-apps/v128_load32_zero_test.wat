;; v128_load32_zero_test.wat - WebAssembly Text Format for v128.load32_zero opcode testing

(module
  ;; Memory: 2 pages (128KB) for comprehensive memory testing
  ;; Page 0: 64KB for basic tests, Page 1: 64KB for boundary tests
  (memory (export "memory") 2)

  ;; Function: test_v128_load32_zero
  ;; Description: Performs v128.load32_zero operation at specified memory offset
  ;; Parameters: One i32 value (memory offset)
  ;; Returns: Four i32 values representing v128 result (little-endian layout)
  (func $test_v128_load32_zero (export "test_v128_load32_zero")
    (param $offset i32)
    (result i32 i32 i32 i32)
    (local $result v128)

    ;; Load 32-bit value from memory at offset into v128 with zero-extension
    ;; v128.load32_zero loads 32 bits and zero-extends to 128 bits
    (local.get $offset)
    (v128.load32_zero)
    (local.set $result)

    ;; Extract v128 result as four i32 values (little-endian format)
    ;; Lane 0: bits 0-31 (contains the loaded 32-bit value)
    ;; Lane 1: bits 32-63 (should be zero due to zero-extension)
    ;; Lane 2: bits 64-95 (should be zero due to zero-extension)
    ;; Lane 3: bits 96-127 (should be zero due to zero-extension)
    (local.get $result)
    (i32x4.extract_lane 0)  ;; Extract lane 0: loaded 32-bit value

    (local.get $result)
    (i32x4.extract_lane 1)  ;; Extract lane 1: should be 0

    (local.get $result)
    (i32x4.extract_lane 2)  ;; Extract lane 2: should be 0

    (local.get $result)
    (i32x4.extract_lane 3)  ;; Extract lane 3: should be 0
  )

  ;; Function: init_memory
  ;; Description: Initialize memory at specified offset with 32-bit value
  ;; Parameters: offset (i32), value (i32)
  ;; Returns: None
  ;; Purpose: Helper function to set up memory for v128.load32_zero testing
  (func $init_memory (export "init_memory")
    (param $offset i32) (param $value i32)

    ;; Store 32-bit value at memory offset (little-endian)
    (local.get $offset)
    (local.get $value)
    (i32.store)
  )

  ;; Function: init_memory_pattern
  ;; Description: Initialize memory region with specific test pattern
  ;; Parameters: base_offset (i32), pattern_type (i32)
  ;; Returns: None
  ;; Purpose: Set up complex memory patterns for comprehensive testing
  (func $init_memory_pattern (export "init_memory_pattern")
    (param $base_offset i32) (param $pattern_type i32)
    (local $i i32)
    (local $value i32)

    ;; Simple pattern selection without br_table
    (local.get $pattern_type)
    (i32.const 0)
    (i32.eq)
    (if
      (then
        ;; Pattern 0: All zeros
        (local.set $i (i32.const 0))
        (loop $zero_loop
          (local.get $base_offset)
          (local.get $i)
          (i32.add)
          (i32.const 0x00000000)
          (i32.store)

          (local.get $i)
          (i32.const 4)
          (i32.add)
          (local.tee $i)
          (i32.const 64)  ;; Initialize 16 values (64 bytes)
          (i32.lt_u)
          (br_if $zero_loop)
        )
        (return)
      )
    )

    (local.get $pattern_type)
    (i32.const 1)
    (i32.eq)
    (if
      (then
        ;; Pattern 1: Maximum values
        (local.set $i (i32.const 0))
        (loop $max_loop
          (local.get $base_offset)
          (local.get $i)
          (i32.add)
          (i32.const 0xFFFFFFFF)
          (i32.store)

          (local.get $i)
          (i32.const 4)
          (i32.add)
          (local.tee $i)
          (i32.const 64)
          (i32.lt_u)
          (br_if $max_loop)
        )
        (return)
      )
    )

    (local.get $pattern_type)
    (i32.const 2)
    (i32.eq)
    (if
      (then
        ;; Pattern 2: Alternating pattern
        (local.set $i (i32.const 0))
        (loop $alt_loop
          (local.get $base_offset)
          (local.get $i)
          (i32.add)

          ;; Alternate between 0xAAAAAAAA and 0x55555555
          (local.get $i)
          (i32.const 8)
          (i32.rem_u)
          (if (result i32)
            (then (i32.const 0xAAAAAAAA))
            (else (i32.const 0x55555555))
          )
          (i32.store)

          (local.get $i)
          (i32.const 4)
          (i32.add)
          (local.tee $i)
          (i32.const 64)
          (i32.lt_u)
          (br_if $alt_loop)
        )
        (return)
      )
    )

    ;; Pattern 3: Sign bit patterns (default)
    (local.set $i (i32.const 0))
    (loop $sign_loop
      (local.get $base_offset)
      (local.get $i)
      (i32.add)

      ;; Alternate between 0x80000000 and 0x7FFFFFFF
      (local.get $i)
      (i32.const 8)
      (i32.rem_u)
      (if (result i32)
        (then (i32.const 0x80000000))  ;; Sign bit set
        (else (i32.const 0x7FFFFFFF))  ;; Maximum positive
      )
      (i32.store)

      (local.get $i)
      (i32.const 4)
      (i32.add)
      (local.tee $i)
      (i32.const 64)
      (i32.lt_u)
      (br_if $sign_loop)
    )
  )

  ;; Function: test_v128_load32_zero_aligned
  ;; Description: Test v128.load32_zero with aligned memory access
  ;; Parameters: base_offset (i32) - must be 4-byte aligned
  ;; Returns: Four i32 values representing v128 result
  ;; Purpose: Specific testing of aligned memory access patterns
  (func $test_v128_load32_zero_aligned (export "test_v128_load32_zero_aligned")
    (param $base_offset i32)
    (result i32 i32 i32 i32)
    (local $result v128)

    ;; Ensure offset is aligned (base_offset & 3 == 0)
    (local.get $base_offset)
    (i32.const 3)
    (i32.and)
    (if
      (then
        ;; If not aligned, return error pattern (all -1)
        (i32.const -1) (i32.const -1) (i32.const -1) (i32.const -1)
        (return)
      )
    )

    ;; Perform aligned v128.load32_zero
    (local.get $base_offset)
    (v128.load32_zero)
    (local.set $result)

    ;; Return v128 as four i32 values
    (local.get $result) (i32x4.extract_lane 0)
    (local.get $result) (i32x4.extract_lane 1)
    (local.get $result) (i32x4.extract_lane 2)
    (local.get $result) (i32x4.extract_lane 3)
  )

  ;; Function: test_v128_load32_zero_unaligned
  ;; Description: Test v128.load32_zero with potentially unaligned memory access
  ;; Parameters: offset (i32) - can be unaligned
  ;; Returns: Four i32 values representing v128 result
  ;; Purpose: Validate unaligned memory access handling
  (func $test_v128_load32_zero_unaligned (export "test_v128_load32_zero_unaligned")
    (param $offset i32)
    (result i32 i32 i32 i32)
    (local $result v128)

    ;; Perform unaligned v128.load32_zero (WASM allows unaligned SIMD loads)
    (local.get $offset)
    (v128.load32_zero)
    (local.set $result)

    ;; Return v128 as four i32 values
    (local.get $result) (i32x4.extract_lane 0)
    (local.get $result) (i32x4.extract_lane 1)
    (local.get $result) (i32x4.extract_lane 2)
    (local.get $result) (i32x4.extract_lane 3)
  )

  ;; Function: get_memory_size
  ;; Description: Get current memory size in pages
  ;; Returns: i32 - current memory size in pages (each page is 64KB)
  ;; Purpose: Helper for boundary testing
  (func $get_memory_size (export "get_memory_size")
    (result i32)
    (memory.size)
  )

  ;; Function: test_boundary_load
  ;; Description: Test v128.load32_zero at memory boundaries
  ;; Parameters: offset_from_end (i32) - bytes from memory end
  ;; Returns: Four i32 values representing v128 result, or error indicators
  ;; Purpose: Test memory boundary conditions
  (func $test_boundary_load (export "test_boundary_load")
    (param $offset_from_end i32)
    (result i32 i32 i32 i32)
    (local $memory_size_bytes i32)
    (local $test_offset i32)
    (local $result v128)

    ;; Calculate memory size in bytes
    (memory.size)
    (i32.const 65536)  ;; 64KB per page
    (i32.mul)
    (local.set $memory_size_bytes)

    ;; Calculate test offset (memory_size - offset_from_end)
    (local.get $memory_size_bytes)
    (local.get $offset_from_end)
    (i32.sub)
    (local.set $test_offset)

    ;; Check if offset + 4 bytes would exceed memory
    (local.get $test_offset)
    (i32.const 4)
    (i32.add)
    (local.get $memory_size_bytes)
    (i32.gt_u)
    (if
      (then
        ;; Out of bounds - return error indicators
        (i32.const -1) (i32.const -1) (i32.const -1) (i32.const -1)
        (return)
      )
    )

    ;; Perform boundary load test
    (local.get $test_offset)
    (v128.load32_zero)
    (local.set $result)

    ;; Return result
    (local.get $result) (i32x4.extract_lane 0)
    (local.get $result) (i32x4.extract_lane 1)
    (local.get $result) (i32x4.extract_lane 2)
    (local.get $result) (i32x4.extract_lane 3)
  )

  ;; Initialize memory with test data on module instantiation
  (start $init_test_data)

  ;; Function: init_test_data
  ;; Description: Initialize memory with default test patterns
  ;; Purpose: Set up memory for immediate testing after module load
  (func $init_test_data
    ;; Initialize some basic test values
    (i32.const 0)   (i32.const 0x12345678) (call $init_memory)  ;; offset 0
    (i32.const 4)   (i32.const 0xABCDEF00) (call $init_memory)  ;; offset 4
    (i32.const 8)   (i32.const 0x55555555) (call $init_memory)  ;; offset 8
    (i32.const 12)  (i32.const 0x80000001) (call $init_memory)  ;; offset 12
    (i32.const 16)  (i32.const 0xFFFFFFFF) (call $init_memory)  ;; offset 16
    (i32.const 20)  (i32.const 0x00000000) (call $init_memory)  ;; offset 20
    (i32.const 24)  (i32.const 0xAAAAAAAA) (call $init_memory)  ;; offset 24
    (i32.const 28)  (i32.const 0x7FFFFFFF) (call $init_memory)  ;; offset 28

    ;; Initialize pattern regions for comprehensive testing
    (i32.const 64)  (i32.const 0) (call $init_memory_pattern)   ;; Zero pattern
    (i32.const 128) (i32.const 1) (call $init_memory_pattern)   ;; Max pattern
    (i32.const 192) (i32.const 2) (call $init_memory_pattern)   ;; Alternating
    (i32.const 256) (i32.const 3) (call $init_memory_pattern)   ;; Sign patterns
  )
)