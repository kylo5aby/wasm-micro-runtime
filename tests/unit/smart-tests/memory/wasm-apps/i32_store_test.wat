;; Enhanced WebAssembly Test Module for i32.store Opcode
;;
;; This module provides comprehensive test functions for validating i32.store
;; operations across different scenarios including basic stores, offset operations,
;; boundary conditions, alignment variations, and error cases.
;;
;; Memory Layout:
;; - Memory size: 1 page (64KB) initial with expansion capability for testing
;; - Test regions distributed across memory to avoid interference
;; - Addresses aligned and unaligned for comprehensive testing
;;
;; Test Functions:
;; - store_i32: Basic i32.store wrapper function (zero offset)
;; - store_i32_offset4: Store with offset 4
;; - store_i32_offset8: Store with offset 8
;; - store_i32_offset16: Store with offset 16
;; - store_i32_large_offset: Store with large offset for overflow testing
;; - load_i32: Helper for memory content validation
;; - get_memory_size: Helper for memory boundary validation

(module
  ;; Memory definition: 1 page (64KB) initial, 2 pages (128KB) maximum
  (memory $mem 1 2)

  ;; Export memory for external access and validation
  (export "memory" (memory $mem))

  ;; Basic i32.store test function (zero offset)
  ;; Parameters: addr (i32), value (i32)
  ;; Returns: none (operation success indicated by no trap)
  (func $store_i32 (param $addr i32) (param $value i32)
    (i32.store
      (local.get $addr)    ;; memory address
      (local.get $value)   ;; i32 value to store
    )
  )
  (export "store_i32" (func $store_i32))

  ;; i32.store with offset 4
  ;; Parameters: addr (i32), value (i32)
  ;; Effective address = addr + 4
  (func $store_i32_offset4 (param $addr i32) (param $value i32)
    (i32.store offset=4
      (local.get $addr)    ;; base address
      (local.get $value)   ;; i32 value to store
    )
  )
  (export "store_i32_offset4" (func $store_i32_offset4))

  ;; i32.store with offset 8
  ;; Parameters: addr (i32), value (i32)
  ;; Effective address = addr + 8
  (func $store_i32_offset8 (param $addr i32) (param $value i32)
    (i32.store offset=8
      (local.get $addr)    ;; base address
      (local.get $value)   ;; i32 value to store
    )
  )
  (export "store_i32_offset8" (func $store_i32_offset8))

  ;; i32.store with offset 16
  ;; Parameters: addr (i32), value (i32)
  ;; Effective address = addr + 16
  (func $store_i32_offset16 (param $addr i32) (param $value i32)
    (i32.store offset=16
      (local.get $addr)    ;; base address
      (local.get $value)   ;; i32 value to store
    )
  )
  (export "store_i32_offset16" (func $store_i32_offset16))

  ;; i32.store with large offset for overflow testing
  ;; Parameters: addr (i32), value (i32)
  ;; Effective address = addr + 0x80000000 (likely to cause overflow)
  (func $store_i32_large_offset (param $addr i32) (param $value i32)
    (i32.store offset=0x80000000
      (local.get $addr)    ;; base address
      (local.get $value)   ;; i32 value to store
    )
  )
  (export "store_i32_large_offset" (func $store_i32_large_offset))

  ;; i32.store with unaligned access (align=0)
  ;; Parameters: addr (i32), value (i32)
  (func $store_i32_unaligned (param $addr i32) (param $value i32)
    (i32.store align=1
      (local.get $addr)    ;; memory address
      (local.get $value)   ;; i32 value to store
    )
  )
  (export "store_i32_unaligned" (func $store_i32_unaligned))

  ;; i32.store with natural alignment (align=4 = 4-byte alignment)
  ;; Parameters: addr (i32), value (i32)
  (func $store_i32_aligned (param $addr i32) (param $value i32)
    (i32.store align=4
      (local.get $addr)    ;; memory address
      (local.get $value)   ;; i32 value to store
    )
  )
  (export "store_i32_aligned" (func $store_i32_aligned))

  ;; Basic i32.load test function for validation
  ;; Parameters: addr (i32)
  ;; Returns: loaded value (i32)
  (func $load_i32 (param $addr i32) (result i32)
    (i32.load
      (local.get $addr)    ;; memory address
    )
  )
  (export "load_i32" (func $load_i32))

  ;; i32.load with offset 4 for validation
  ;; Parameters: addr (i32)
  ;; Returns: loaded value (i32)
  (func $load_i32_offset4 (param $addr i32) (result i32)
    (i32.load offset=4
      (local.get $addr)    ;; base address
    )
  )
  (export "load_i32_offset4" (func $load_i32_offset4))

  ;; Store/Load combination test for data integrity
  ;; Parameters: addr (i32), value (i32)
  ;; Returns: loaded value (i32) - should match stored value
  (func $store_load_test (param $addr i32) (param $value i32) (result i32)
    ;; Store the value
    (i32.store
      (local.get $addr)
      (local.get $value)
    )
    ;; Load and return the value
    (i32.load
      (local.get $addr)
    )
  )
  (export "store_load_test" (func $store_load_test))

  ;; Pattern store test for endianness verification
  ;; Stores a known pattern and returns individual bytes
  ;; Parameters: addr (i32)
  ;; Returns: success flag (i32)
  (func $pattern_store_test (param $addr i32) (result i32)
    (local $pattern i32)
    (local $byte0 i32)
    (local $byte1 i32)
    (local $byte2 i32)
    (local $byte3 i32)

    ;; Store known pattern 0x12345678
    (local.set $pattern (i32.const 0x12345678))
    (i32.store
      (local.get $addr)
      (local.get $pattern)
    )

    ;; Load individual bytes to verify little-endian storage
    (local.set $byte0 (i32.load8_u (local.get $addr)))
    (local.set $byte1 (i32.load8_u (i32.add (local.get $addr) (i32.const 1))))
    (local.set $byte2 (i32.load8_u (i32.add (local.get $addr) (i32.const 2))))
    (local.set $byte3 (i32.load8_u (i32.add (local.get $addr) (i32.const 3))))

    ;; Verify little-endian byte order: 0x78, 0x56, 0x34, 0x12
    (i32.and
      (i32.and
        (i32.eq (local.get $byte0) (i32.const 0x78))
        (i32.eq (local.get $byte1) (i32.const 0x56))
      )
      (i32.and
        (i32.eq (local.get $byte2) (i32.const 0x34))
        (i32.eq (local.get $byte3) (i32.const 0x12))
      )
    )
  )
  (export "pattern_store_test" (func $pattern_store_test))

  ;; Memory boundary validation helper
  ;; Returns current memory size in bytes
  (func $get_memory_size (result i32)
    (i32.mul
      (memory.size)
      (i32.const 65536)  ;; 64KB per page
    )
  )
  (export "get_memory_size" (func $get_memory_size))

  ;; Sequential store test for memory layout validation
  ;; Stores values at consecutive addresses and verifies no interference
  ;; Parameters: base_addr (i32)
  ;; Returns: success flag (i32)
  (func $sequential_store_test (param $base_addr i32) (result i32)
    (local $val1 i32)
    (local $val2 i32)
    (local $val3 i32)
    (local $addr1 i32)
    (local $addr2 i32)
    (local $addr3 i32)

    ;; Set up test addresses (4-byte intervals for i32)
    (local.set $addr1 (local.get $base_addr))
    (local.set $addr2 (i32.add (local.get $base_addr) (i32.const 4)))
    (local.set $addr3 (i32.add (local.get $base_addr) (i32.const 8)))

    ;; Store distinct values
    (local.set $val1 (i32.const 0x11111111))
    (local.set $val2 (i32.const 0x22222222))
    (local.set $val3 (i32.const 0x33333333))

    (i32.store (local.get $addr1) (local.get $val1))
    (i32.store (local.get $addr2) (local.get $val2))
    (i32.store (local.get $addr3) (local.get $val3))

    ;; Verify all values are stored correctly
    (i32.and
      (i32.and
        (i32.eq (i32.load (local.get $addr1)) (local.get $val1))
        (i32.eq (i32.load (local.get $addr2)) (local.get $val2))
      )
      (i32.eq (i32.load (local.get $addr3)) (local.get $val3))
    )
  )
  (export "sequential_store_test" (func $sequential_store_test))

  ;; Zero value store test
  ;; Parameters: addr (i32)
  ;; Returns: success flag (i32)
  (func $zero_store_test (param $addr i32) (result i32)
    ;; First store non-zero value to ensure memory is modified
    (i32.store (local.get $addr) (i32.const 0xFFFFFFFF))

    ;; Then store zero value
    (i32.store (local.get $addr) (i32.const 0))

    ;; Verify zero value was stored
    (i32.eqz (i32.load (local.get $addr)))
  )
  (export "zero_store_test" (func $zero_store_test))
)