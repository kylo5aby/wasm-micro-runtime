;; v128_load64_zero_test.wat - WebAssembly Text Format for v128.load64_zero opcode testing

(module
  ;; Memory: 2 pages (128KB) for comprehensive memory testing
  ;; Page 0: 64KB for basic tests, Page 1: 64KB for boundary tests
  (memory (export "memory") 2)

  ;; Function: test_v128_load64_zero
  ;; Description: Performs v128.load64_zero operation at specified memory offset
  ;; Parameters: One i32 value (memory offset)
  ;; Returns: Four i32 values representing v128 result (little-endian layout)
  (func $test_v128_load64_zero (export "test_v128_load64_zero")
    (param $offset i32)
    (result i32 i32 i32 i32)
    (local $result v128)

    ;; Load 64-bit value from memory at offset into v128 with zero-extension
    ;; v128.load64_zero loads 64 bits and zero-extends to 128 bits
    (local.get $offset)
    (v128.load64_zero)
    (local.set $result)

    ;; Extract v128 result as four i32 values (little-endian format)
    ;; Lane 0: bits 0-31 (lower 32 bits of loaded 64-bit value)
    ;; Lane 1: bits 32-63 (upper 32 bits of loaded 64-bit value)
    ;; Lane 2: bits 64-95 (should be zero due to zero-extension)
    ;; Lane 3: bits 96-127 (should be zero due to zero-extension)
    (local.get $result)
    (i32x4.extract_lane 0)  ;; Extract lane 0: lower 32 bits of loaded value

    (local.get $result)
    (i32x4.extract_lane 1)  ;; Extract lane 1: upper 32 bits of loaded value

    (local.get $result)
    (i32x4.extract_lane 2)  ;; Extract lane 2: should be 0

    (local.get $result)
    (i32x4.extract_lane 3)  ;; Extract lane 3: should be 0
  )

  ;; Function: init_memory_64
  ;; Description: Initialize memory at specified offset with 64-bit value
  ;; Parameters: offset (i32), value_low (i32), value_high (i32)
  ;; Returns: None
  ;; Purpose: Helper function to set up memory for v128.load64_zero testing
  (func $init_memory_64 (export "init_memory_64")
    (param $offset i32) (param $value_low i32) (param $value_high i32)

    ;; Store lower 32 bits at offset (little-endian)
    (local.get $offset)
    (local.get $value_low)
    (i32.store)

    ;; Store upper 32 bits at offset + 4 (little-endian)
    (local.get $offset)
    (i32.const 4)
    (i32.add)
    (local.get $value_high)
    (i32.store)
  )

  ;; Function: init_memory_32
  ;; Description: Initialize memory at specified offset with 32-bit value (compatibility)
  ;; Parameters: offset (i32), value (i32)
  ;; Returns: None
  ;; Purpose: Helper function for 32-bit memory initialization
  (func $init_memory_32 (export "init_memory_32")
    (param $offset i32) (param $value i32)

    ;; Store 32-bit value at memory offset (little-endian)
    (local.get $offset)
    (local.get $value)
    (i32.store)
  )

  ;; Function: test_v128_load64_zero_with_offset
  ;; Description: Test v128.load64_zero with memarg offset parameter
  ;; Parameters: base_addr (i32)
  ;; Returns: Four i32 values representing v128 result
  (func $test_v128_load64_zero_with_offset (export "test_v128_load64_zero_with_offset")
    (param $base_addr i32)
    (result i32 i32 i32 i32)
    (local $result v128)

    ;; Load 64-bit value from memory at base_addr + 8 (memarg offset = 8)
    (local.get $base_addr)
    (v128.load64_zero offset=8)
    (local.set $result)

    ;; Extract v128 result as four i32 values
    (local.get $result)
    (i32x4.extract_lane 0)

    (local.get $result)
    (i32x4.extract_lane 1)

    (local.get $result)
    (i32x4.extract_lane 2)

    (local.get $result)
    (i32x4.extract_lane 3)
  )

  ;; Function: test_alignment_scenarios
  ;; Description: Test various memory alignment scenarios for v128.load64_zero
  ;; Parameters: base_addr (i32), offset (i32)
  ;; Returns: Four i32 values representing v128 result
  (func $test_alignment_scenarios (export "test_alignment_scenarios")
    (param $base_addr i32) (param $alignment_offset i32)
    (result i32 i32 i32 i32)
    (local $result v128)
    (local $effective_addr i32)

    ;; Calculate effective address for alignment testing
    (local.get $base_addr)
    (local.get $alignment_offset)
    (i32.add)
    (local.set $effective_addr)

    ;; Load 64-bit value from calculated address
    (local.get $effective_addr)
    (v128.load64_zero)
    (local.set $result)

    ;; Extract v128 result as four i32 values
    (local.get $result)
    (i32x4.extract_lane 0)

    (local.get $result)
    (i32x4.extract_lane 1)

    (local.get $result)
    (i32x4.extract_lane 2)

    (local.get $result)
    (i32x4.extract_lane 3)
  )

  ;; Function: get_memory_size
  ;; Description: Get current memory size in pages for boundary testing
  ;; Returns: Current memory size in pages
  (func $get_memory_size (export "get_memory_size")
    (result i32)
    (memory.size)
  )

  ;; Function: clear_memory_region
  ;; Description: Clear memory region with zeros for clean testing
  ;; Parameters: offset (i32), size (i32)
  (func $clear_memory_region (export "clear_memory_region")
    (param $offset i32) (param $size i32)
    (local $i i32)
    (local $end i32)

    (local.get $offset)
    (local.get $size)
    (i32.add)
    (local.set $end)

    (local.get $offset)
    (local.set $i)

    (loop $clear_loop
      (local.get $i)
      (local.get $end)
      (i32.lt_u)
      (if
        (then
          (local.get $i)
          (i32.const 0)
          (i32.store8)
          (local.get $i)
          (i32.const 1)
          (i32.add)
          (local.set $i)
          (br $clear_loop)
        )
      )
    )
  )
)