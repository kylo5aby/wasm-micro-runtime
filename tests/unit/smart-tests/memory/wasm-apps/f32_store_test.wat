;; Enhanced WebAssembly Test Module for f32.store Opcode
;;
;; This module provides comprehensive test functions for validating f32.store
;; operations across different scenarios including basic stores, aligned/unaligned
;; access, special floating-point values, boundary conditions, and error cases.
;;
;; Memory Layout:
;; - Memory size: 1 page (64KB) initial with expansion capability for testing
;; - Test regions distributed across memory to avoid interference
;; - Addresses aligned and unaligned for comprehensive f32.store testing
;;
;; Test Functions:
;; - store_f32: Basic f32.store wrapper function (zero offset)
;; - load_f32: Helper for memory content validation and verification
;; - store_f32_aligned: Store with 4-byte alignment
;; - store_f32_unaligned: Store with 1-byte alignment (unaligned access)
;; - store_f32_offset4: Store with offset 4
;; - store_f32_offset8: Store with offset 8
;; - get_memory_size: Helper for memory boundary validation

(module
  ;; Memory definition: 1 page (64KB) initial, 2 pages (128KB) maximum
  (memory $mem 1 2)

  ;; Export memory for external access and validation
  (export "memory" (memory $mem))

  ;; Basic f32.store test function (zero offset)
  ;; Parameters: addr (i32), value (f32)
  ;; Returns: none (operation success indicated by no trap)
  (func $store_f32 (param $addr i32) (param $value f32)
    (f32.store
      (local.get $addr)    ;; memory address
      (local.get $value)   ;; f32 value to store
    )
  )
  (export "store_f32" (func $store_f32))

  ;; f32.load helper function for verification
  ;; Parameters: addr (i32)
  ;; Returns: f32 (loaded value from memory)
  (func $load_f32 (param $addr i32) (result f32)
    (f32.load
      (local.get $addr)    ;; memory address
    )
  )
  (export "load_f32" (func $load_f32))

  ;; f32.store with natural alignment (align=4 = 4-byte alignment)
  ;; Parameters: addr (i32), value (f32)
  (func $store_f32_aligned (param $addr i32) (param $value f32)
    (f32.store align=4
      (local.get $addr)    ;; memory address (should be 4-byte aligned)
      (local.get $value)   ;; f32 value to store
    )
  )
  (export "store_f32_aligned" (func $store_f32_aligned))

  ;; f32.store with unaligned access (align=1)
  ;; Parameters: addr (i32), value (f32)
  (func $store_f32_unaligned (param $addr i32) (param $value f32)
    (f32.store align=1
      (local.get $addr)    ;; memory address (can be unaligned)
      (local.get $value)   ;; f32 value to store
    )
  )
  (export "store_f32_unaligned" (func $store_f32_unaligned))

  ;; f32.store with offset 4
  ;; Parameters: addr (i32), value (f32)
  ;; Effective address = addr + 4
  (func $store_f32_offset4 (param $addr i32) (param $value f32)
    (f32.store offset=4
      (local.get $addr)    ;; base address
      (local.get $value)   ;; f32 value to store
    )
  )
  (export "store_f32_offset4" (func $store_f32_offset4))

  ;; f32.store with offset 8
  ;; Parameters: addr (i32), value (f32)
  ;; Effective address = addr + 8
  (func $store_f32_offset8 (param $addr i32) (param $value f32)
    (f32.store offset=8
      (local.get $addr)    ;; base address
      (local.get $value)   ;; f32 value to store
    )
  )
  (export "store_f32_offset8" (func $store_f32_offset8))

  ;; f32.store with large offset for boundary testing
  ;; Parameters: addr (i32), value (f32)
  ;; Effective address = addr + 0x10000 (near 64KB boundary)
  (func $store_f32_boundary_offset (param $addr i32) (param $value f32)
    (f32.store offset=0x10000
      (local.get $addr)    ;; base address
      (local.get $value)   ;; f32 value to store
    )
  )
  (export "store_f32_boundary_offset" (func $store_f32_boundary_offset))

  ;; Helper: Get current memory size in pages
  ;; Returns: i32 (memory size in pages, multiply by 65536 for bytes)
  (func $get_memory_size (result i32)
    (memory.size)
  )
  (export "get_memory_size" (func $get_memory_size))

  ;; Helper: Store special f32 values (NaN, infinity, zero)
  ;; Parameters: addr (i32), special_type (i32)
  ;; special_type: 0=quiet_NaN, 1=signaling_NaN, 2=+inf, 3=-inf, 4=+zero, 5=-zero
  (func $store_special_f32 (param $addr i32) (param $special_type i32)
    (local $value f32)

    ;; Select special value based on type parameter
    (local.set $value
      (block (result f32)
        (block $case5
          (block $case4
            (block $case3
              (block $case2
                (block $case1
                  (block $case0
                    (br_table $case0 $case1 $case2 $case3 $case4 $case5 (local.get $special_type))
                  )
                  ;; case 0: quiet NaN (0x7FC00000)
                  (f32.reinterpret_i32 (i32.const 0x7FC00000))
                  (br 5)
                )
                ;; case 1: signaling NaN (0x7F800001)
                (f32.reinterpret_i32 (i32.const 0x7F800001))
                (br 4)
              )
              ;; case 2: positive infinity (0x7F800000)
              (f32.reinterpret_i32 (i32.const 0x7F800000))
              (br 3)
            )
            ;; case 3: negative infinity (0xFF800000)
            (f32.reinterpret_i32 (i32.const 0xFF800000))
            (br 2)
          )
          ;; case 4: positive zero (0x00000000)
          (f32.reinterpret_i32 (i32.const 0x00000000))
          (br 1)
        )
        ;; case 5: negative zero (0x80000000)
        (f32.reinterpret_i32 (i32.const 0x80000000))
      )
    )

    ;; Store the special value
    (f32.store
      (local.get $addr)
      (local.get $value)
    )
  )
  (export "store_special_f32" (func $store_special_f32))

  ;; Helper: Store denormal f32 values
  ;; Parameters: addr (i32), denormal_type (i32)
  ;; denormal_type: 0=smallest_pos, 1=largest_pos, 2=smallest_neg, 3=largest_neg
  (func $store_denormal_f32 (param $addr i32) (param $denormal_type i32)
    (local $value f32)

    ;; Select denormal value based on type parameter
    (local.set $value
      (block (result f32)
        (block $case3
          (block $case2
            (block $case1
              (block $case0
                (br_table $case0 $case1 $case2 $case3 (local.get $denormal_type))
              )
              ;; case 0: smallest positive denormal (0x00000001)
              (f32.reinterpret_i32 (i32.const 0x00000001))
              (br 3)
            )
            ;; case 1: largest positive denormal (0x007FFFFF)
            (f32.reinterpret_i32 (i32.const 0x007FFFFF))
            (br 2)
          )
          ;; case 2: smallest negative denormal (0x80000001)
          (f32.reinterpret_i32 (i32.const 0x80000001))
          (br 1)
        )
        ;; case 3: largest negative denormal (0x807FFFFF)
        (f32.reinterpret_i32 (i32.const 0x807FFFFF))
      )
    )

    ;; Store the denormal value
    (f32.store
      (local.get $addr)
      (local.get $value)
    )
  )
  (export "store_denormal_f32" (func $store_denormal_f32))

  ;; Helper: Test out-of-bounds access by trying to store beyond memory limit
  ;; Parameters: addr (i32), value (f32)
  ;; This function is expected to trap when addr+4 > memory_size
  (func $test_out_of_bounds_store (param $addr i32) (param $value f32)
    (f32.store
      (local.get $addr)    ;; potentially invalid address
      (local.get $value)   ;; f32 value to store
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

  ;; Helper: Get raw bits of f32 value at address
  ;; Parameters: addr (i32)
  ;; Returns: i32 (raw bit representation of f32)
  (func $get_f32_bits (param $addr i32) (result i32)
    (i32.reinterpret_f32
      (f32.load (local.get $addr))
    )
  )
  (export "get_f32_bits" (func $get_f32_bits))

  ;; Helper: Store f32 value from raw bits
  ;; Parameters: addr (i32), bits (i32)
  (func $store_f32_from_bits (param $addr i32) (param $bits i32)
    (f32.store
      (local.get $addr)
      (f32.reinterpret_i32 (local.get $bits))
    )
  )
  (export "store_f32_from_bits" (func $store_f32_from_bits))

  ;; Memory initialization section (if needed for specific test data)
  ;; Initialize first 64 bytes to zero for predictable testing
  (func $init_memory
    (call $clear_memory (i32.const 0) (i32.const 64))
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

    ;; Calculate maximum valid address for f32 (memory_size - 4)
    (local.set $max_valid_addr
      (i32.sub (local.get $memory_bytes) (i32.const 4)))

    ;; Try to store at maximum valid address
    (f32.store
      (local.get $max_valid_addr)
      (f32.const 999.999)
    )

    ;; If we reach here, store succeeded
    (i32.const 1)
  )
  (export "test_boundary_store" (func $test_boundary_store))
)