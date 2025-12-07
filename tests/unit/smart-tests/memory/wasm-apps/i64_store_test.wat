;; Enhanced WebAssembly Test Module for i64.store Opcode
;;
;; This module provides comprehensive test functions for validating i64.store
;; operations across different scenarios including basic stores, boundary conditions,
;; alignment variations, and error cases.
;;
;; Memory Layout:
;; - Memory size: 1 page (64KB) initial with expansion capability for testing
;; - Test regions distributed across memory to avoid interference
;; - Addresses aligned and unaligned for comprehensive testing
;;
;; Test Functions:
;; - store_i64: Basic i64.store wrapper function
;; - load_i64: Helper for memory content validation
;; - get_memory_size: Helper for memory boundary validation

(module
  ;; Memory definition: 1 page (64KB) initial, 2 pages (128KB) maximum
  (memory $mem 1 2)

  ;; Export memory for external access and validation
  (export "memory" (memory $mem))

  ;; Basic i64.store test function
  ;; Parameters: addr (i32), value_low (i32), value_high (i32)
  ;; The i64 value is passed as two i32 parameters due to WASM interface limitations
  ;; Returns: none (operation success indicated by no trap)
  (func $store_i64 (param $addr i32) (param $value_low i32) (param $value_high i32)
    (i64.store
      (local.get $addr)                    ;; memory address
      (i64.or                              ;; reconstruct i64 from two i32 parts
        (i64.extend_i32_u (local.get $value_low))          ;; lower 32 bits
        (i64.shl
          (i64.extend_i32_u (local.get $value_high))       ;; upper 32 bits
          (i64.const 32)                   ;; shift left 32 positions
        )
      )
    )
  )
  (export "store_i64" (func $store_i64))

  ;; i64.store with offset 8 for alignment testing
  ;; Parameters: addr (i32), value_low (i32), value_high (i32)
  ;; Effective address = addr + 8
  (func $store_i64_offset8 (param $addr i32) (param $value_low i32) (param $value_high i32)
    (i64.store offset=8
      (local.get $addr)                    ;; base address
      (i64.or                              ;; reconstruct i64 from two i32 parts
        (i64.extend_i32_u (local.get $value_low))          ;; lower 32 bits
        (i64.shl
          (i64.extend_i32_u (local.get $value_high))       ;; upper 32 bits
          (i64.const 32)
        )
      )
    )
  )
  (export "store_i64_offset8" (func $store_i64_offset8))

  ;; i64.store with offset 16 for boundary testing
  ;; Parameters: addr (i32), value_low (i32), value_high (i32)
  ;; Effective address = addr + 16
  (func $store_i64_offset16 (param $addr i32) (param $value_low i32) (param $value_high i32)
    (i64.store offset=16
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
  (export "store_i64_offset16" (func $store_i64_offset16))

  ;; i64.store with large offset for overflow testing
  ;; Parameters: addr (i32), value_low (i32), value_high (i32)
  ;; Effective address = addr + 32760 (close to page boundary for 64KB page)
  (func $store_i64_large_offset (param $addr i32) (param $value_low i32) (param $value_high i32)
    (i64.store offset=32760
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
  (export "store_i64_large_offset" (func $store_i64_large_offset))

  ;; Helper function: i64.load for memory content validation
  ;; Parameters: addr (i32)
  ;; Returns: loaded i64 value split into two i32 parts (low, high)
  (func $load_i64 (param $addr i32) (result i32 i32)
    (local $value i64)
    ;; Load the i64 value from memory
    (local.set $value (i64.load (local.get $addr)))

    ;; Return lower 32 bits
    (i32.wrap_i64 (local.get $value))

    ;; Return upper 32 bits
    (i32.wrap_i64 (i64.shr_u (local.get $value) (i64.const 32)))
  )
  (export "load_i64" (func $load_i64))

  ;; Helper function: i64.load with offset 8
  ;; Parameters: addr (i32)
  ;; Returns: loaded i64 value split into two i32 parts (low, high)
  (func $load_i64_offset8 (param $addr i32) (result i32 i32)
    (local $value i64)
    (local.set $value (i64.load offset=8 (local.get $addr)))

    (i32.wrap_i64 (local.get $value))
    (i32.wrap_i64 (i64.shr_u (local.get $value) (i64.const 32)))
  )
  (export "load_i64_offset8" (func $load_i64_offset8))

  ;; Helper function: Get current memory size in bytes
  ;; Returns: memory size in bytes (pages * 65536)
  (func $get_memory_size (result i32)
    (i32.mul
      (memory.size)     ;; get memory size in pages
      (i32.const 65536) ;; convert pages to bytes (1 page = 64KB = 65536 bytes)
    )
  )
  (export "get_memory_size" (func $get_memory_size))

  ;; Helper function: Grow memory by one page for boundary testing
  ;; Returns: previous memory size in pages, or -1 on failure
  (func $grow_memory (result i32)
    (memory.grow (i32.const 1))
  )
  (export "grow_memory" (func $grow_memory))

  ;; Test function for zero value storage validation
  ;; Parameters: addr (i32)
  ;; Stores zero i64 value at specified address
  (func $store_zero_i64 (param $addr i32)
    (i64.store
      (local.get $addr)
      (i64.const 0)     ;; zero value
    )
  )
  (export "store_zero_i64" (func $store_zero_i64))

  ;; Test function for maximum positive i64 value storage
  ;; Parameters: addr (i32)
  ;; Stores INT64_MAX (0x7FFFFFFFFFFFFFFF) at specified address
  (func $store_max_i64 (param $addr i32)
    (i64.store
      (local.get $addr)
      (i64.const 0x7FFFFFFFFFFFFFFF)  ;; INT64_MAX
    )
  )
  (export "store_max_i64" (func $store_max_i64))

  ;; Test function for minimum i64 value storage
  ;; Parameters: addr (i32)
  ;; Stores INT64_MIN (0x8000000000000000) at specified address
  (func $store_min_i64 (param $addr i32)
    (i64.store
      (local.get $addr)
      (i64.const 0x8000000000000000)  ;; INT64_MIN
    )
  )
  (export "store_min_i64" (func $store_min_i64))

  ;; Test function for alternating bit pattern storage
  ;; Parameters: addr (i32)
  ;; Stores 0x5555555555555555 pattern
  (func $store_alternating_pattern (param $addr i32)
    (i64.store
      (local.get $addr)
      (i64.const 0x5555555555555555)  ;; alternating bit pattern
    )
  )
  (export "store_alternating_pattern" (func $store_alternating_pattern))

  ;; Test function for inverse alternating bit pattern storage
  ;; Parameters: addr (i32)
  ;; Stores 0xAAAAAAAAAAAAAAAA pattern
  (func $store_inverse_pattern (param $addr i32)
    (i64.store
      (local.get $addr)
      (i64.const 0xAAAAAAAAAAAAAAAA)  ;; inverse alternating pattern
    )
  )
  (export "store_inverse_pattern" (func $store_inverse_pattern))

  ;; Test function for boundary condition: store at calculated address
  ;; Parameters: base_addr (i32), offset (i32), value_low (i32), value_high (i32)
  ;; Effective address = base_addr + offset
  (func $store_i64_calculated_addr (param $base_addr i32) (param $offset i32)
                                   (param $value_low i32) (param $value_high i32)
    (i64.store
      (i32.add (local.get $base_addr) (local.get $offset))  ;; calculated address
      (i64.or
        (i64.extend_i32_u (local.get $value_low))
        (i64.shl
          (i64.extend_i32_u (local.get $value_high))
          (i64.const 32)
        )
      )
    )
  )
  (export "store_i64_calculated_addr" (func $store_i64_calculated_addr))

  ;; Test function: Fill memory region with pattern for testing
  ;; Parameters: start_addr (i32), size (i32), pattern_low (i32), pattern_high (i32)
  (func $fill_memory_pattern (param $start_addr i32) (param $size i32)
                             (param $pattern_low i32) (param $pattern_high i32)
    (local $addr i32)
    (local $end_addr i32)
    (local $pattern i64)

    (local.set $addr (local.get $start_addr))
    (local.set $end_addr (i32.add (local.get $start_addr) (local.get $size)))
    (local.set $pattern
      (i64.or
        (i64.extend_i32_u (local.get $pattern_low))
        (i64.shl
          (i64.extend_i32_u (local.get $pattern_high))
          (i64.const 32)
        )
      )
    )

    ;; Fill memory with pattern in 8-byte increments
    (loop $fill_loop
      (i64.store (local.get $addr) (local.get $pattern))
      (local.set $addr (i32.add (local.get $addr) (i32.const 8)))
      (br_if $fill_loop (i32.lt_u (local.get $addr) (local.get $end_addr)))
    )
  )
  (export "fill_memory_pattern" (func $fill_memory_pattern))
)