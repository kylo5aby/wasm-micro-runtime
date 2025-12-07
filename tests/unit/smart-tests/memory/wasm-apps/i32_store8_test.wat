;; Enhanced WebAssembly Test Module for i32.store8 Opcode
;;
;; This module provides comprehensive test functions for validating i32.store8
;; operations across different scenarios including basic stores, value truncation,
;; boundary conditions, offset operations, and error cases.
;;
;; Memory Layout:
;; - Memory size: 1 page (64KB) initial with expansion capability for testing
;; - Test regions distributed across memory to avoid interference
;; - Addresses aligned and unaligned for comprehensive byte-level testing
;;
;; Test Functions:
;; - store_and_load_byte: Store i32 as byte and load back for verification
;; - store_with_offset4: Store with offset 4 for address calculation testing
;; - store_with_offset8: Store with offset 8 for address calculation testing
;; - store_with_offset0: Store with zero offset for baseline testing
;; - test_out_of_bounds_store: Test out-of-bounds access handling
;; - get_memory_size: Helper for memory boundary validation

(module
  ;; Memory definition: 1 page (64KB) initial, 2 pages (128KB) maximum
  (memory $mem 1 2)

  ;; Export memory for external access and validation
  (export "memory" (memory $mem))

  ;; Store i32 value as byte and immediately load back for verification
  ;; Parameters: addr (i32), value (i32)
  ;; Returns: i32 (the byte value that was stored, zero-extended)
  ;; Operation: i32.store8 followed by i32.load8_u for verification
  (func $store_and_load_byte (param $addr i32) (param $value i32) (result i32)
    ;; Store the i32 value as a single byte (truncated to 8 bits)
    (i32.store8
      (local.get $addr)    ;; memory address
      (local.get $value)   ;; i32 value to store (will be truncated to 8 bits)
    )

    ;; Load the byte back as unsigned i32 for verification
    (i32.load8_u
      (local.get $addr)    ;; same memory address
    )
  )
  (export "store_and_load_byte" (func $store_and_load_byte))

  ;; i32.store8 with offset 4 for address calculation testing
  ;; Parameters: addr (i32), value (i32)
  ;; Returns: i32 (the byte value that was stored at effective address)
  ;; Effective address = addr + 4
  (func $store_with_offset4 (param $addr i32) (param $value i32) (result i32)
    ;; Store with immediate offset 4
    (i32.store8 offset=4
      (local.get $addr)    ;; base address
      (local.get $value)   ;; i32 value to store as byte
    )

    ;; Load back from effective address for verification
    (i32.load8_u offset=4
      (local.get $addr)    ;; same base address
    )
  )
  (export "store_with_offset4" (func $store_with_offset4))

  ;; i32.store8 with offset 8 for address calculation testing
  ;; Parameters: addr (i32), value (i32)
  ;; Returns: i32 (the byte value that was stored at effective address)
  ;; Effective address = addr + 8
  (func $store_with_offset8 (param $addr i32) (param $value i32) (result i32)
    ;; Store with immediate offset 8
    (i32.store8 offset=8
      (local.get $addr)    ;; base address
      (local.get $value)   ;; i32 value to store as byte
    )

    ;; Load back from effective address for verification
    (i32.load8_u offset=8
      (local.get $addr)    ;; same base address
    )
  )
  (export "store_with_offset8" (func $store_with_offset8))

  ;; i32.store8 with zero offset (baseline test)
  ;; Parameters: addr (i32), value (i32)
  ;; Returns: i32 (the byte value that was stored)
  ;; Effective address = addr + 0
  (func $store_with_offset0 (param $addr i32) (param $value i32) (result i32)
    ;; Store with zero immediate offset
    (i32.store8 offset=0
      (local.get $addr)    ;; address
      (local.get $value)   ;; i32 value to store as byte
    )

    ;; Load back for verification
    (i32.load8_u offset=0
      (local.get $addr)    ;; same address
    )
  )
  (export "store_with_offset0" (func $store_with_offset0))

  ;; Test out-of-bounds store8 access for error handling validation
  ;; Parameters: addr (i32), value (i32)
  ;; Returns: i32 (should trap before returning)
  ;; This function deliberately attempts out-of-bounds access to test trap behavior
  (func $test_out_of_bounds_store (param $addr i32) (param $value i32) (result i32)
    ;; Attempt to store to potentially out-of-bounds address
    ;; This should trap if the address is beyond memory bounds
    (i32.store8
      (local.get $addr)    ;; potentially invalid address
      (local.get $value)   ;; value to store
    )

    ;; If we reach here, the store succeeded (within bounds)
    ;; Return the stored value for verification
    (i32.load8_u
      (local.get $addr)
    )
  )
  (export "test_out_of_bounds_store" (func $test_out_of_bounds_store))

  ;; Store byte at specific address without loading back
  ;; Parameters: addr (i32), value (i32)
  ;; Returns: none
  ;; Useful for testing store operations where loading back is not desired
  (func $store_byte_only (param $addr i32) (param $value i32)
    (i32.store8
      (local.get $addr)    ;; memory address
      (local.get $value)   ;; i32 value to store as byte
    )
  )
  (export "store_byte_only" (func $store_byte_only))

  ;; Load byte value from specific address
  ;; Parameters: addr (i32)
  ;; Returns: i32 (byte value zero-extended to i32)
  ;; Useful for verification after separate store operations
  (func $load_byte_only (param $addr i32) (result i32)
    (i32.load8_u
      (local.get $addr)    ;; memory address to load from
    )
  )
  (export "load_byte_only" (func $load_byte_only))

  ;; Test value truncation behavior with specific patterns
  ;; Parameters: addr (i32), value (i32)
  ;; Returns: i32 (demonstrates truncation of high-order bits)
  (func $test_truncation (param $addr i32) (param $value i32) (result i32)
    ;; Store the full i32 value (only lowest 8 bits will be stored)
    (i32.store8
      (local.get $addr)
      (local.get $value)
    )

    ;; Load back and compare with expected truncated value
    ;; The result should equal (value & 0xFF)
    (i32.load8_u
      (local.get $addr)
    )
  )
  (export "test_truncation" (func $test_truncation))

  ;; Sequential byte storage test for address validation
  ;; Parameters: base_addr (i32), val1 (i32), val2 (i32), val3 (i32)
  ;; Returns: i32 (combined validation result)
  ;; Stores 3 bytes sequentially and validates each one
  (func $test_sequential_bytes (param $base i32) (param $val1 i32) (param $val2 i32) (param $val3 i32) (result i32)
    (local $result i32)

    ;; Store first byte
    (i32.store8
      (local.get $base)
      (local.get $val1)
    )

    ;; Store second byte (base + 1)
    (i32.store8
      (i32.add (local.get $base) (i32.const 1))
      (local.get $val2)
    )

    ;; Store third byte (base + 2)
    (i32.store8
      (i32.add (local.get $base) (i32.const 2))
      (local.get $val3)
    )

    ;; Load and combine all three bytes for verification
    ;; Result format: byte3 << 16 | byte2 << 8 | byte1
    (local.set $result
      (i32.or
        (i32.or
          (i32.load8_u (local.get $base))                                    ;; byte1
          (i32.shl (i32.load8_u (i32.add (local.get $base) (i32.const 1))) (i32.const 8))  ;; byte2 << 8
        )
        (i32.shl (i32.load8_u (i32.add (local.get $base) (i32.const 2))) (i32.const 16))   ;; byte3 << 16
      )
    )

    (local.get $result)
  )
  (export "test_sequential_bytes" (func $test_sequential_bytes))

  ;; Get current memory size in pages for boundary testing
  ;; Returns: i32 (memory size in pages)
  (func $get_memory_size (result i32)
    (memory.size)
  )
  (export "get_memory_size" (func $get_memory_size))

  ;; Get memory size in bytes for boundary address calculation
  ;; Returns: i32 (memory size in bytes)
  (func $get_memory_size_bytes (result i32)
    (i32.mul
      (memory.size)           ;; size in pages
      (i32.const 65536)       ;; 64KB per page
    )
  )
  (export "get_memory_size_bytes" (func $get_memory_size_bytes))

  ;; Test alignment-independent byte storage
  ;; Parameters: none
  ;; Returns: i32 (success indicator: 1 for success, 0 for failure)
  ;; Tests storage at various alignments to verify byte-level access
  (func $test_alignment_independence (result i32)
    (local $success i32)
    (local.set $success (i32.const 1))  ;; Assume success

    ;; Test storage at odd addresses (unaligned for larger types)
    (i32.store8 (i32.const 1) (i32.const 0xAA))
    (i32.store8 (i32.const 3) (i32.const 0xBB))
    (i32.store8 (i32.const 5) (i32.const 0xCC))
    (i32.store8 (i32.const 7) (i32.const 0xDD))

    ;; Verify stored values
    (if (i32.ne (i32.load8_u (i32.const 1)) (i32.const 0xAA))
      (then (local.set $success (i32.const 0)))
    )
    (if (i32.ne (i32.load8_u (i32.const 3)) (i32.const 0xBB))
      (then (local.set $success (i32.const 0)))
    )
    (if (i32.ne (i32.load8_u (i32.const 5)) (i32.const 0xCC))
      (then (local.set $success (i32.const 0)))
    )
    (if (i32.ne (i32.load8_u (i32.const 7)) (i32.const 0xDD))
      (then (local.set $success (i32.const 0)))
    )

    (local.get $success)
  )
  (export "test_alignment_independence" (func $test_alignment_independence))
)