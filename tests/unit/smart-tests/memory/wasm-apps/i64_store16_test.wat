;; Enhanced WebAssembly Test Module for i64.store16 Opcode
;;
;; This module provides comprehensive test functions for validating i64.store16
;; operations across different scenarios including basic stores, value truncation,
;; boundary conditions, offset operations, and error cases.
;;
;; Memory Layout:
;; - Memory size: 1 page (64KB) initial with expansion capability for testing
;; - Test regions distributed across memory to avoid interference
;; - Addresses aligned and unaligned for comprehensive 16-bit level testing
;;
;; Test Functions:
;; - store_i64_short: Store i64 as 16-bit value (main test function)
;; - load_short: Load 16-bit value as i32 for verification
;; - store_i64_short_oob: Test out-of-bounds access handling
;; - store_with_offset4: Store with offset 4 for address calculation testing
;; - store_with_offset8: Store with offset 8 for address calculation testing
;; - get_memory_size: Helper for memory boundary validation

(module
  ;; Memory definition: 1 page (64KB) initial, 2 pages (128KB) maximum
  (memory $mem 1 2)

  ;; Export memory for external access and validation
  (export "memory" (memory $mem))

  ;; Store i64 value as 16-bit value without returning value
  ;; Parameters: addr (i32), value_low (i32), value_high (i32)
  ;; Operation: Combines low/high i32s into i64, then stores lower 16 bits as short
  (func $store_i64_short (param $addr i32) (param $value_low i32) (param $value_high i32)
    ;; Combine the two i32 parameters into a single i64 value
    ;; value_high goes in upper 32 bits, value_low in lower 32 bits
    (i64.store16
      (local.get $addr)                             ;; memory address
      (i64.or
        (i64.extend_i32_u (local.get $value_low))   ;; lower 32 bits
        (i64.shl
          (i64.extend_i32_u (local.get $value_high)) ;; upper 32 bits
          (i64.const 32)                            ;; shift left by 32 bits
        )
      )
    )
  )
  (export "store_i64_short" (func $store_i64_short))

  ;; Load 16-bit value from memory and return as i32 for verification
  ;; Parameters: addr (i32)
  ;; Returns: i32 (the 16-bit value loaded, zero-extended)
  (func $load_short (param $addr i32) (result i32)
    (i32.load16_u
      (local.get $addr)
    )
  )
  (export "load_short" (func $load_short))

  ;; Store i64 value as 16-bit value with potential out-of-bounds access
  ;; This function is designed to test boundary conditions and trap behavior
  ;; Parameters: addr (i32), value_low (i32), value_high (i32)
  ;; May trap if address is out-of-bounds
  (func $store_i64_short_oob (param $addr i32) (param $value_low i32) (param $value_high i32)
    ;; Same operation as store_i64_short but for testing bounds
    (i64.store16
      (local.get $addr)
      (i64.or
        (i64.extend_i32_u (local.get $value_low))
        (i64.shl
          (i64.extend_i32_u (local.get $value_high))
          (i64.const 32)
        )
      )
    )
  )
  (export "store_i64_short_oob" (func $store_i64_short_oob))

  ;; Store i64 value as 16-bit value with offset 4 for address calculation testing
  ;; Parameters: base_addr (i32), value_low (i32), value_high (i32)
  ;; Operation: Stores at base_addr + 4
  (func $store_with_offset4 (param $base_addr i32) (param $value_low i32) (param $value_high i32)
    (i64.store16 offset=4
      (local.get $base_addr)
      (i64.or
        (i64.extend_i32_u (local.get $value_low))
        (i64.shl
          (i64.extend_i32_u (local.get $value_high))
          (i64.const 32)
        )
      )
    )
  )
  (export "store_with_offset4" (func $store_with_offset4))

  ;; Store i64 value as 16-bit value with offset 8 for address calculation testing
  ;; Parameters: base_addr (i32), value_low (i32), value_high (i32)
  ;; Operation: Stores at base_addr + 8
  (func $store_with_offset8 (param $base_addr i32) (param $value_low i32) (param $value_high i32)
    (i64.store16 offset=8
      (local.get $base_addr)
      (i64.or
        (i64.extend_i32_u (local.get $value_low))
        (i64.shl
          (i64.extend_i32_u (local.get $value_high))
          (i64.const 32)
        )
      )
    )
  )
  (export "store_with_offset8" (func $store_with_offset8))

  ;; Load 16-bit value with offset 4 for verification
  ;; Parameters: base_addr (i32)
  ;; Returns: i32 (16-bit value at base_addr + 4)
  (func $load_short_offset4 (param $base_addr i32) (result i32)
    (i32.load16_u offset=4
      (local.get $base_addr)
    )
  )
  (export "load_short_offset4" (func $load_short_offset4))

  ;; Load 16-bit value with offset 8 for verification
  ;; Parameters: base_addr (i32)
  ;; Returns: i32 (16-bit value at base_addr + 8)
  (func $load_short_offset8 (param $base_addr i32) (result i32)
    (i32.load16_u offset=8
      (local.get $base_addr)
    )
  )
  (export "load_short_offset8" (func $load_short_offset8))

  ;; Store multiple 16-bit values sequentially for bulk testing
  ;; Parameters: start_addr (i32), value1_low (i32), value1_high (i32),
  ;;            value2_low (i32), value2_high (i32), value3_low (i32), value3_high (i32)
  (func $store_multiple_shorts (param $start_addr i32)
                               (param $value1_low i32) (param $value1_high i32)
                               (param $value2_low i32) (param $value2_high i32)
                               (param $value3_low i32) (param $value3_high i32)
    ;; Store first value at start_addr
    (i64.store16
      (local.get $start_addr)
      (i64.or
        (i64.extend_i32_u (local.get $value1_low))
        (i64.shl (i64.extend_i32_u (local.get $value1_high)) (i64.const 32))
      )
    )

    ;; Store second value at start_addr + 2
    (i64.store16
      (i32.add (local.get $start_addr) (i32.const 2))
      (i64.or
        (i64.extend_i32_u (local.get $value2_low))
        (i64.shl (i64.extend_i32_u (local.get $value2_high)) (i64.const 32))
      )
    )

    ;; Store third value at start_addr + 4
    (i64.store16
      (i32.add (local.get $start_addr) (i32.const 4))
      (i64.or
        (i64.extend_i32_u (local.get $value3_low))
        (i64.shl (i64.extend_i32_u (local.get $value3_high)) (i64.const 32))
      )
    )
  )
  (export "store_multiple_shorts" (func $store_multiple_shorts))

  ;; Store i64 value as 16-bit with unaligned address testing
  ;; Parameters: addr (i32), value_low (i32), value_high (i32)
  ;; Operation: Tests alignment behavior (WebAssembly allows unaligned access)
  (func $store_unaligned (param $addr i32) (param $value_low i32) (param $value_high i32)
    (i64.store16 align=1
      (local.get $addr)
      (i64.or
        (i64.extend_i32_u (local.get $value_low))
        (i64.shl
          (i64.extend_i32_u (local.get $value_high))
          (i64.const 32)
        )
      )
    )
  )
  (export "store_unaligned" (func $store_unaligned))

  ;; Get current memory size in pages
  ;; Returns: i32 (memory size in pages, 1 page = 64KB)
  (func $get_memory_pages (result i32)
    (memory.size)
  )
  (export "get_memory_pages" (func $get_memory_pages))

  ;; Get current memory size in bytes for boundary testing
  ;; Returns: i32 (memory size in bytes)
  (func $get_memory_size (result i32)
    (i32.mul
      (memory.size)    ;; pages
      (i32.const 65536) ;; bytes per page
    )
  )
  (export "get_memory_size" (func $get_memory_size))

  ;; Zero-fill memory region for clean testing
  ;; Parameters: addr (i32), length (i32)
  ;; Operation: Sets length bytes starting at addr to zero
  (func $zero_memory (param $addr i32) (param $length i32)
    (memory.fill
      (local.get $addr)    ;; destination address
      (i32.const 0)        ;; fill value (zero)
      (local.get $length)  ;; number of bytes
    )
  )
  (export "zero_memory" (func $zero_memory))

  ;; Pattern-fill memory region for testing data integrity
  ;; Parameters: addr (i32), length (i32), pattern (i32)
  ;; Operation: Sets length bytes starting at addr to pattern & 0xFF
  (func $pattern_fill_memory (param $addr i32) (param $length i32) (param $pattern i32)
    (memory.fill
      (local.get $addr)          ;; destination address
      (local.get $pattern)       ;; fill value (pattern byte)
      (local.get $length)        ;; number of bytes
    )
  )
  (export "pattern_fill_memory" (func $pattern_fill_memory))

  ;; Test boundary store operation - stores exactly at memory size minus 2
  ;; This function helps test edge cases near memory boundaries
  ;; Returns: i32 (1 if successful, 0 if trapped)
  (func $test_boundary_store (param $value_low i32) (param $value_high i32) (result i32)
    (local $addr i32)
    (local $mem_size i32)

    ;; Calculate address = memory_size - 2 (last valid address for 16-bit store)
    (local.set $mem_size (call $get_memory_size))
    (local.set $addr (i32.sub (local.get $mem_size) (i32.const 2)))

    ;; Attempt store operation
    (i64.store16
      (local.get $addr)
      (i64.or
        (i64.extend_i32_u (local.get $value_low))
        (i64.shl (i64.extend_i32_u (local.get $value_high)) (i64.const 32))
      )
    )

    ;; Return success indicator
    (i32.const 1)
  )
  (export "test_boundary_store" (func $test_boundary_store))

  ;; Validate stored value matches expected truncated result
  ;; Parameters: addr (i32), expected_low (i32), expected_high (i32)
  ;; Returns: i32 (1 if match, 0 if mismatch)
  (func $validate_stored_short (param $addr i32) (param $expected_low i32) (param $expected_high i32) (result i32)
    (local $loaded_value i32)
    (local $expected_truncated i32)

    ;; Load the stored 16-bit value
    (local.set $loaded_value (call $load_short (local.get $addr)))

    ;; Calculate expected 16-bit value from input parameters
    ;; We only care about the lower 16 bits of the combined i64
    (local.set $expected_truncated
      (i32.and (local.get $expected_low) (i32.const 0xFFFF))
    )

    ;; Compare loaded vs expected
    (i32.eq (local.get $loaded_value) (local.get $expected_truncated))
  )
  (export "validate_stored_short" (func $validate_stored_short))
)