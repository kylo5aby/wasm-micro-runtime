;; Enhanced WebAssembly Test Module for f64.store Opcode
;;
;; This module provides comprehensive test functions for validating f64.store
;; operations across different scenarios including basic stores, aligned/unaligned
;; access, special double-precision floating-point values, boundary conditions, and error cases.
;;
;; Memory Layout:
;; - Memory size: 1 page (64KB) initial with expansion capability for testing
;; - Test regions distributed across memory to avoid interference
;; - Addresses aligned and unaligned for comprehensive f64.store testing
;;
;; Test Functions:
;; - store_f64: Basic f64.store wrapper function (zero offset)
;; - load_f64: Helper for memory content validation and verification
;; - store_f64_aligned: Store with 8-byte alignment
;; - store_f64_unaligned: Store with 1-byte alignment (unaligned access)
;; - store_f64_offset8: Store with offset 8
;; - get_memory_size: Helper for memory boundary validation

(module
  ;; Memory definition: 1 page (64KB) initial, 2 pages (128KB) maximum
  (memory $mem 1 2)

  ;; Export memory for external access and validation
  (export "memory" (memory $mem))

  ;; Basic f64.store test function (zero offset)
  ;; Parameters: addr (i32), value (f64)
  ;; Returns: none (operation success indicated by no trap)
  (func $store_f64 (param $addr i32) (param $value f64)
    (f64.store
      (local.get $addr)    ;; memory address
      (local.get $value)   ;; f64 value to store
    )
  )
  (export "store_f64" (func $store_f64))

  ;; f64.load helper function for verification
  ;; Parameters: addr (i32)
  ;; Returns: f64 (loaded value from memory)
  (func $load_f64 (param $addr i32) (result f64)
    (f64.load
      (local.get $addr)    ;; memory address
    )
  )
  (export "load_f64" (func $load_f64))

  ;; f64.store with natural alignment (align=8 = 8-byte alignment)
  ;; Parameters: addr (i32), value (f64)
  (func $store_f64_aligned (param $addr i32) (param $value f64)
    (f64.store align=8
      (local.get $addr)    ;; memory address (should be 8-byte aligned)
      (local.get $value)   ;; f64 value to store
    )
  )
  (export "store_f64_aligned" (func $store_f64_aligned))

  ;; f64.store with unaligned access (align=1)
  ;; Parameters: addr (i32), value (f64)
  (func $store_f64_unaligned (param $addr i32) (param $value f64)
    (f64.store align=1
      (local.get $addr)    ;; memory address (can be unaligned)
      (local.get $value)   ;; f64 value to store
    )
  )
  (export "store_f64_unaligned" (func $store_f64_unaligned))

  ;; f64.store with offset 8
  ;; Parameters: addr (i32), value (f64)
  ;; Effective address = addr + 8
  (func $store_f64_offset8 (param $addr i32) (param $value f64)
    (f64.store offset=8
      (local.get $addr)    ;; base address
      (local.get $value)   ;; f64 value to store
    )
  )
  (export "store_f64_offset8" (func $store_f64_offset8))

  ;; f64.store with offset 16
  ;; Parameters: addr (i32), value (f64)
  ;; Effective address = addr + 16
  (func $store_f64_offset16 (param $addr i32) (param $value f64)
    (f64.store offset=16
      (local.get $addr)    ;; base address
      (local.get $value)   ;; f64 value to store
    )
  )
  (export "store_f64_offset16" (func $store_f64_offset16))

  ;; f64.store with large offset for boundary testing
  ;; Parameters: addr (i32), value (f64)
  ;; Effective address = addr + 0x10000 (near 64KB boundary)
  (func $store_f64_boundary_offset (param $addr i32) (param $value f64)
    (f64.store offset=0x10000
      (local.get $addr)    ;; base address
      (local.get $value)   ;; f64 value to store
    )
  )
  (export "store_f64_boundary_offset" (func $store_f64_boundary_offset))

  ;; Helper: Get current memory size in pages
  ;; Returns: i32 (memory size in pages, multiply by 65536 for bytes)
  (func $get_memory_size (result i32)
    (memory.size)
  )
  (export "get_memory_size" (func $get_memory_size))

  ;; Helper: Store special f64 values (NaN, infinity, zero)
  ;; Parameters: addr (i32), special_type (i32)
  ;; special_type: 0=quiet_NaN, 1=signaling_NaN, 2=+inf, 3=-inf, 4=+zero, 5=-zero, 6=max_finite, 7=min_normal
  (func $store_special_f64 (param $addr i32) (param $special_type i32)
    (local $value f64)

    ;; Select special value based on type parameter
    (local.set $value
      (block (result f64)
        (block $case7
          (block $case6
            (block $case5
              (block $case4
                (block $case3
                  (block $case2
                    (block $case1
                      (block $case0
                        (br_table $case0 $case1 $case2 $case3 $case4 $case5 $case6 $case7 (local.get $special_type))
                      )
                      ;; case 0: quiet NaN (0x7FF8000000000000)
                      (f64.reinterpret_i64 (i64.const 0x7FF8000000000000))
                      (br 7)
                    )
                    ;; case 1: signaling NaN (0x7FF0000000000001)
                    (f64.reinterpret_i64 (i64.const 0x7FF0000000000001))
                    (br 6)
                  )
                  ;; case 2: positive infinity (0x7FF0000000000000)
                  (f64.reinterpret_i64 (i64.const 0x7FF0000000000000))
                  (br 5)
                )
                ;; case 3: negative infinity (0xFFF0000000000000)
                (f64.reinterpret_i64 (i64.const 0xFFF0000000000000))
                (br 4)
              )
              ;; case 4: positive zero (0x0000000000000000)
              (f64.reinterpret_i64 (i64.const 0x0000000000000000))
              (br 3)
            )
            ;; case 5: negative zero (0x8000000000000000)
            (f64.reinterpret_i64 (i64.const 0x8000000000000000))
            (br 2)
          )
          ;; case 6: maximum finite value (0x7FEFFFFFFFFFFFFF)
          (f64.reinterpret_i64 (i64.const 0x7FEFFFFFFFFFFFFF))
          (br 1)
        )
        ;; case 7: minimum normal value (0x0010000000000000)
        (f64.reinterpret_i64 (i64.const 0x0010000000000000))
      )
    )

    ;; Store the special value
    (f64.store
      (local.get $addr)
      (local.get $value)
    )
  )
  (export "store_special_f64" (func $store_special_f64))

  ;; Helper: Store denormal f64 values (subnormal numbers)
  ;; Parameters: addr (i32), denormal_type (i32)
  ;; denormal_type: 0=smallest_pos, 1=largest_pos, 2=smallest_neg, 3=largest_neg
  (func $store_denormal_f64 (param $addr i32) (param $denormal_type i32)
    (local $value f64)

    ;; Select denormal value based on type parameter
    (local.set $value
      (block (result f64)
        (block $case3
          (block $case2
            (block $case1
              (block $case0
                (br_table $case0 $case1 $case2 $case3 (local.get $denormal_type))
              )
              ;; case 0: smallest positive denormal (0x0000000000000001)
              (f64.reinterpret_i64 (i64.const 0x0000000000000001))
              (br 3)
            )
            ;; case 1: largest positive denormal (0x000FFFFFFFFFFFFF)
            (f64.reinterpret_i64 (i64.const 0x000FFFFFFFFFFFFF))
            (br 2)
          )
          ;; case 2: smallest negative denormal (0x8000000000000001)
          (f64.reinterpret_i64 (i64.const 0x8000000000000001))
          (br 1)
        )
        ;; case 3: largest negative denormal (0x800FFFFFFFFFFFFF)
        (f64.reinterpret_i64 (i64.const 0x800FFFFFFFFFFFFF))
      )
    )

    ;; Store the denormal value
    (f64.store
      (local.get $addr)
      (local.get $value)
    )
  )
  (export "store_denormal_f64" (func $store_denormal_f64))

  ;; Helper: Test out-of-bounds access by trying to store beyond memory limit
  ;; Parameters: addr (i32), value (f64)
  ;; This function is expected to trap when addr+8 > memory_size
  (func $test_out_of_bounds_store (param $addr i32) (param $value f64)
    (f64.store
      (local.get $addr)    ;; potentially invalid address
      (local.get $value)   ;; f64 value to store
    )
  )
  (export "test_out_of_bounds_store" (func $test_out_of_bounds_store))

  ;; Helper: Clear memory region for clean testing
  ;; Parameters: start_addr (i32), size (i32)
  (func $clear_memory (param $start_addr i32) (param $size i32)
    (local $end_addr i32)
    (local $current_addr i32)

    (local.set $end_addr
      (i32.add (local.get $start_addr) (local.get $size)))
    (local.set $current_addr (local.get $start_addr))

    (loop $clear_loop
      ;; Store zero at current address
      (i32.store8
        (local.get $current_addr)
        (i32.const 0)
      )

      ;; Increment address
      (local.set $current_addr
        (i32.add (local.get $current_addr) (i32.const 1)))

      ;; Continue loop if not at end
      (br_if $clear_loop
        (i32.lt_u (local.get $current_addr) (local.get $end_addr)))
    )
  )
  (export "clear_memory" (func $clear_memory))

  ;; Helper: Get raw bits of f64 value at address
  ;; Parameters: addr (i32)
  ;; Returns: i64 (raw bit representation of f64)
  (func $get_f64_bits (param $addr i32) (result i64)
    (i64.reinterpret_f64
      (f64.load (local.get $addr))
    )
  )
  (export "get_f64_bits" (func $get_f64_bits))

  ;; Helper: Store f64 value from raw bits
  ;; Parameters: addr (i32), bits (i64)
  (func $store_f64_from_bits (param $addr i32) (param $bits i64)
    (f64.store
      (local.get $addr)
      (f64.reinterpret_i64 (local.get $bits))
    )
  )
  (export "store_f64_from_bits" (func $store_f64_from_bits))

  ;; Memory initialization section (if needed for specific test data)
  ;; Initialize first 128 bytes to zero for predictable testing
  (func $init_memory
    (call $clear_memory (i32.const 0) (i32.const 128))
  )
  (export "init_memory" (func $init_memory))

  ;; Test boundary scenarios - attempt to store at maximum valid address
  ;; Returns: 1 if successful, 0 if trapped
  (func $test_boundary_store (result i32)
    (local $memory_bytes i32)
    (local $max_valid_addr i32)

    ;; Calculate memory size in bytes
    (local.set $memory_bytes
      (i32.mul (memory.size) (i32.const 65536)))

    ;; Calculate maximum valid address for f64 (memory_size - 8)
    (local.set $max_valid_addr
      (i32.sub (local.get $memory_bytes) (i32.const 8)))

    ;; Try to store at maximum valid address
    (f64.store
      (local.get $max_valid_addr)
      (f64.const 999.999999999)
    )

    ;; If we reach here, store succeeded
    (i32.const 1)
  )
  (export "test_boundary_store" (func $test_boundary_store))

  ;; Helper: Test precision preservation for double values
  ;; Parameters: addr (i32)
  ;; Stores and retrieves a high-precision double value
  (func $test_precision_preservation (param $addr i32) (result f64)
    (local $high_precision_value f64)

    ;; High precision double: π with maximum precision
    (local.set $high_precision_value (f64.const 3.141592653589793238))

    ;; Store the value
    (f64.store
      (local.get $addr)
      (local.get $high_precision_value)
    )

    ;; Load and return the value
    (f64.load (local.get $addr))
  )
  (export "test_precision_preservation" (func $test_precision_preservation))

  ;; Helper: Store multiple f64 values in sequence
  ;; Parameters: base_addr (i32)
  ;; Stores a sequence of f64 values at consecutive 8-byte addresses
  (func $store_sequence_f64 (param $base_addr i32)
    ;; Store π at base_addr
    (f64.store
      (local.get $base_addr)
      (f64.const 3.141592653589793)
    )

    ;; Store e at base_addr + 8
    (f64.store
      (i32.add (local.get $base_addr) (i32.const 8))
      (f64.const 2.718281828459045)
    )

    ;; Store √2 at base_addr + 16
    (f64.store
      (i32.add (local.get $base_addr) (i32.const 16))
      (f64.const 1.414213562373095)
    )

    ;; Store φ (golden ratio) at base_addr + 24
    (f64.store
      (i32.add (local.get $base_addr) (i32.const 24))
      (f64.const 1.618033988749895)
    )
  )
  (export "store_sequence_f64" (func $store_sequence_f64))
)