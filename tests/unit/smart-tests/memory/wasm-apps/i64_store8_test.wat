;; Enhanced WebAssembly Test Module for i64.store8 Opcode
;;
;; This module provides comprehensive test functions for validating i64.store8
;; operations across different scenarios including basic stores, value truncation,
;; boundary conditions, offset operations, and error cases.
;;
;; Memory Layout:
;; - Memory size: 1 page (64KB) initial with expansion capability for testing
;; - Test regions distributed across memory to avoid interference
;; - Addresses aligned and unaligned for comprehensive byte-level testing
;;
;; Test Functions:
;; - store_i64_byte: Store i64 as byte (main test function)
;; - load_byte: Load byte as i32 for verification
;; - store_i64_byte_oob: Test out-of-bounds access handling
;; - store_with_offset4: Store with offset 4 for address calculation testing
;; - store_with_offset8: Store with offset 8 for address calculation testing
;; - get_memory_size: Helper for memory boundary validation

(module
  ;; Memory definition: 1 page (64KB) initial, 2 pages (128KB) maximum
  (memory $mem 1 2)

  ;; Export memory for external access and validation
  (export "memory" (memory $mem))

  ;; Store i64 value as byte without returning value
  ;; Parameters: addr (i32), value_low (i32), value_high (i32)
  ;; Operation: Combines low/high i32s into i64, then stores lower 8 bits as byte
  (func $store_i64_byte (param $addr i32) (param $value_low i32) (param $value_high i32)
    ;; Combine the two i32 parameters into a single i64 value
    ;; value_high goes in upper 32 bits, value_low in lower 32 bits
    (i64.store8
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
  (export "store_i64_byte" (func $store_i64_byte))

  ;; Load byte from memory and return as i32 for verification
  ;; Parameters: addr (i32)
  ;; Returns: i32 (the byte value loaded, zero-extended)
  (func $load_byte (param $addr i32) (result i32)
    (i32.load8_u
      (local.get $addr)    ;; memory address
    )
  )
  (export "load_byte" (func $load_byte))

  ;; Store i64 as byte with out-of-bounds testing capability
  ;; This function is designed to test memory bounds violations
  ;; Parameters: addr (i32), value_low (i32), value_high (i32)
  (func $store_i64_byte_oob (param $addr i32) (param $value_low i32) (param $value_high i32)
    ;; Combine the two i32 parameters into a single i64 value
    ;; This may trigger bounds checking if addr is out of range
    (i64.store8
      (local.get $addr)                             ;; memory address (potentially out-of-bounds)
      (i64.or
        (i64.extend_i32_u (local.get $value_low))   ;; lower 32 bits
        (i64.shl
          (i64.extend_i32_u (local.get $value_high)) ;; upper 32 bits
          (i64.const 32)                            ;; shift left by 32 bits
        )
      )
    )
  )
  (export "store_i64_byte_oob" (func $store_i64_byte_oob))

  ;; i64.store8 with offset 4 for address calculation testing
  ;; Parameters: addr (i32), value_low (i32), value_high (i32)
  ;; Returns: i32 (the byte value that was stored at effective address)
  ;; Effective address = addr + 4
  (func $store_with_offset4 (param $addr i32) (param $value_low i32) (param $value_high i32) (result i32)
    ;; Store with immediate offset 4
    (i64.store8 offset=4
      (local.get $addr)                             ;; base address
      (i64.or
        (i64.extend_i32_u (local.get $value_low))   ;; lower 32 bits
        (i64.shl
          (i64.extend_i32_u (local.get $value_high)) ;; upper 32 bits
          (i64.const 32)                            ;; shift left by 32 bits
        )
      )
    )

    ;; Load the byte back from effective address (addr + 4) for verification
    (i32.load8_u offset=4
      (local.get $addr)    ;; base address
    )
  )
  (export "store_with_offset4" (func $store_with_offset4))

  ;; i64.store8 with offset 8 for address calculation testing
  ;; Parameters: addr (i32), value_low (i32), value_high (i32)
  ;; Returns: i32 (the byte value that was stored at effective address)
  ;; Effective address = addr + 8
  (func $store_with_offset8 (param $addr i32) (param $value_low i32) (param $value_high i32) (result i32)
    ;; Store with immediate offset 8
    (i64.store8 offset=8
      (local.get $addr)                             ;; base address
      (i64.or
        (i64.extend_i32_u (local.get $value_low))   ;; lower 32 bits
        (i64.shl
          (i64.extend_i32_u (local.get $value_high)) ;; upper 32 bits
          (i64.const 32)                            ;; shift left by 32 bits
        )
      )
    )

    ;; Load the byte back from effective address (addr + 8) for verification
    (i32.load8_u offset=8
      (local.get $addr)    ;; base address
    )
  )
  (export "store_with_offset8" (func $store_with_offset8))

  ;; i64.store8 with zero offset for baseline testing
  ;; Parameters: addr (i32), value_low (i32), value_high (i32)
  ;; Returns: i32 (the byte value that was stored)
  ;; Effective address = addr + 0 (same as no offset)
  (func $store_with_offset0 (param $addr i32) (param $value_low i32) (param $value_high i32) (result i32)
    ;; Store with explicit zero offset
    (i64.store8 offset=0
      (local.get $addr)                             ;; base address
      (i64.or
        (i64.extend_i32_u (local.get $value_low))   ;; lower 32 bits
        (i64.shl
          (i64.extend_i32_u (local.get $value_high)) ;; upper 32 bits
          (i64.const 32)                            ;; shift left by 32 bits
        )
      )
    )

    ;; Load the byte back for verification
    (i32.load8_u offset=0
      (local.get $addr)    ;; base address
    )
  )
  (export "store_with_offset0" (func $store_with_offset0))

  ;; Test value truncation with various i64 bit patterns
  ;; Parameters: addr (i32), value_low (i32), value_high (i32)
  ;; Returns: i32 (the byte value after truncation)
  (func $test_truncation (param $addr i32) (param $value_low i32) (param $value_high i32) (result i32)
    ;; Store the full i64 value, which will be truncated to 8 bits
    (i64.store8
      (local.get $addr)
      (i64.or
        (i64.extend_i32_u (local.get $value_low))
        (i64.shl
          (i64.extend_i32_u (local.get $value_high))
          (i64.const 32)
        )
      )
    )

    ;; Return the truncated byte value
    (i32.load8_u
      (local.get $addr)
    )
  )
  (export "test_truncation" (func $test_truncation))

  ;; Test boundary conditions at specific memory addresses
  ;; Parameters: addr (i32), value_low (i32), value_high (i32)
  ;; Returns: i32 (success indicator: 1 if store succeeded, 0 if failed)
  (func $test_boundary_store (param $addr i32) (param $value_low i32) (param $value_high i32) (result i32)
    ;; Attempt to store at the given address
    ;; This function helps test boundary conditions
    (i64.store8
      (local.get $addr)
      (i64.or
        (i64.extend_i32_u (local.get $value_low))
        (i64.shl
          (i64.extend_i32_u (local.get $value_high))
          (i64.const 32)
        )
      )
    )

    ;; If we reach this point, the store succeeded
    ;; Return the stored value as confirmation
    (i32.load8_u
      (local.get $addr)
    )
  )
  (export "test_boundary_store" (func $test_boundary_store))

  ;; Get current memory size in pages for boundary testing
  ;; Returns: i32 (memory size in pages)
  (func $get_memory_size_pages (result i32)
    (memory.size)
  )
  (export "get_memory_size_pages" (func $get_memory_size_pages))

  ;; Get current memory size in bytes for boundary testing
  ;; Returns: i32 (memory size in bytes)
  (func $get_memory_size_bytes (result i32)
    ;; Multiply memory size in pages by 65536 (bytes per page)
    (i32.mul
      (memory.size)
      (i32.const 65536)
    )
  )
  (export "get_memory_size_bytes" (func $get_memory_size_bytes))

  ;; Memory grow function for dynamic memory testing
  ;; Parameters: pages (i32) - number of pages to grow by
  ;; Returns: i32 (previous memory size in pages, or -1 if grow failed)
  (func $grow_memory (param $pages i32) (result i32)
    (memory.grow (local.get $pages))
  )
  (export "grow_memory" (func $grow_memory))

  ;; Helper function to test bit patterns preservation
  ;; Tests specific bit patterns to validate byte-level storage accuracy
  ;; Parameters: addr (i32), pattern (i32) - 8-bit pattern to test
  ;; Returns: i32 (1 if pattern preserved correctly, 0 otherwise)
  (func $test_bit_pattern (param $addr i32) (param $pattern i32) (result i32)
    ;; Store the pattern as lower byte of an i64 with upper bits set differently
    (i64.store8
      (local.get $addr)
      (i64.or
        (i64.extend_i32_u (local.get $pattern))     ;; lower 8 bits: our pattern
        (i64.const 0xDEADBEEFCAFE0000)             ;; upper 56 bits: different pattern
      )
    )

    ;; Load back and check if pattern is preserved
    (local.get $pattern)
    (i32.load8_u (local.get $addr))
    (i32.eq)  ;; Compare expected pattern with loaded value
  )
  (export "test_bit_pattern" (func $test_bit_pattern))

  ;; Test zero value storage
  ;; Parameters: addr (i32)
  ;; Returns: i32 (the stored zero value)
  (func $test_zero_store (param $addr i32) (result i32)
    ;; Store zero i64 value
    (i64.store8
      (local.get $addr)
      (i64.const 0)
    )

    ;; Return the loaded value (should be 0)
    (i32.load8_u (local.get $addr))
  )
  (export "test_zero_store" (func $test_zero_store))

  ;; Test maximum i64 value storage (all bits set)
  ;; Parameters: addr (i32)
  ;; Returns: i32 (the stored byte value, should be 0xFF)
  (func $test_max_store (param $addr i32) (result i32)
    ;; Store maximum i64 value (all 64 bits set to 1)
    (i64.store8
      (local.get $addr)
      (i64.const -1)  ;; -1 in i64 is 0xFFFFFFFFFFFFFFFF
    )

    ;; Return the loaded byte (should be 0xFF)
    (i32.load8_u (local.get $addr))
  )
  (export "test_max_store" (func $test_max_store))
)