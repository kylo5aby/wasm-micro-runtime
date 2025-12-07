;; Error Test Module for memory.copy Opcode
;;
;; This module provides test functions specifically designed to trigger
;; error conditions and validate proper error handling in memory.copy operations.
;;
;; Error Test Categories:
;; - Stack underflow simulation
;; - Invalid memory access patterns
;; - Boundary overflow testing
;; - Resource exhaustion scenarios

(module
  ;; Small memory for boundary testing: 1 page (64KB)
  (memory $mem 1 1)
  (export "memory" (memory $mem))

  ;; Function that will cause runtime error - invalid module for stack underflow testing
  ;; This function is designed to be called to test runtime behavior
  (func $test_stack_underflow
    ;; This is a valid function, runtime stack underflow must be tested differently
    (memory.copy
      (i32.const 0)      ;; destination
      (i32.const 16)     ;; source
      (i32.const 8)      ;; size
    )
  )
  (export "test_stack_underflow" (func $test_stack_underflow))

  ;; Function that attempts out-of-bounds destination access
  ;; Memory size is 1 page (65536 bytes), this tries to write beyond that
  (func $test_dest_out_of_bounds
    (memory.copy
      (i32.const 65520)   ;; destination near end of memory
      (i32.const 0)       ;; valid source
      (i32.const 32)      ;; size causes overflow (65520 + 32 > 65536)
    )
  )
  (export "test_dest_out_of_bounds" (func $test_dest_out_of_bounds))

  ;; Function that attempts out-of-bounds source access
  (func $test_src_out_of_bounds
    (memory.copy
      (i32.const 0)       ;; valid destination
      (i32.const 65520)   ;; source near end of memory
      (i32.const 32)      ;; size causes overflow (65520 + 32 > 65536)
    )
  )
  (export "test_src_out_of_bounds" (func $test_src_out_of_bounds))

  ;; Function that attempts copy with maximum values to trigger overflow
  (func $test_integer_overflow
    (memory.copy
      (i32.const 0xFFFFFFF0)   ;; Large destination offset
      (i32.const 0)            ;; Valid source
      (i32.const 0x20)         ;; Size that causes offset + size overflow
    )
  )
  (export "test_integer_overflow" (func $test_integer_overflow))

  ;; Function that attempts copy with both offsets out of bounds
  (func $test_both_out_of_bounds
    (memory.copy
      (i32.const 0x20000)      ;; destination beyond memory (> 65536)
      (i32.const 0x30000)      ;; source beyond memory (> 65536)
      (i32.const 16)
    )
  )
  (export "test_both_out_of_bounds" (func $test_both_out_of_bounds))

  ;; Function that tests maximum size parameter
  (func $test_max_size_copy
    (memory.copy
      (i32.const 0)            ;; valid destination
      (i32.const 0)            ;; valid source (same as dest)
      (i32.const 0xFFFFFFFF)   ;; maximum i32 value as size
    )
  )
  (export "test_max_size_copy" (func $test_max_size_copy))

  ;; Function to test negative-appearing offsets (large positive values)
  (func $test_negative_like_offsets
    (memory.copy
      (i32.const 0x80000000)   ;; Large positive value (appears negative in signed)
      (i32.const 0)
      (i32.const 16)
    )
  )
  (export "test_negative_like_offsets" (func $test_negative_like_offsets))
)