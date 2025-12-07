;; Enhanced WebAssembly Test Module for i32.store16 Opcode
;;
;; This module provides comprehensive test functions for validating i32.store16
;; operations across different scenarios including basic stores, offset operations,
;; value truncation, boundary conditions, and error cases.
;;
;; Memory Layout:
;; - Memory size: 1 page (64KB) initial with expansion capability for testing
;; - Test regions distributed across memory to avoid interference
;; - Addresses aligned for 16-bit store operations
;;
;; i32.store16 Behavior:
;; - Stores lower 16 bits of i32 value to memory (truncation)
;; - Uses 2-byte memory alignment requirements
;; - Performs bounds checking for address + 2 bytes
;; - Immediate offset added to base address for effective address calculation
;;
;; Test Functions:
;; - store16_value: Basic i32.store16 wrapper function (zero offset)
;; - store16_with_offset: Store with immediate offset 4
;; - load16_value: Helper for memory content validation (16-bit loads)
;; - get_memory_size: Helper for memory boundary validation

(module
  ;; Memory definition: 1 page (64KB) initial, 2 pages (128KB) maximum
  (memory $mem 1 2)

  ;; Export memory for external access and validation
  (export "memory" (memory $mem))

  ;; Basic i32.store16 test function (zero offset)
  ;; Parameters: addr (i32), value (i32)
  ;; Behavior: Stores lower 16 bits of value at memory[addr]
  ;; Returns: none (operation success indicated by no trap)
  (func $store16_value (param $addr i32) (param $value i32)
    (i32.store16
      (local.get $addr)    ;; memory address
      (local.get $value)   ;; i32 value to store (lower 16 bits used)
    )
  )
  (export "store16_value" (func $store16_value))

  ;; i32.store16 with immediate offset 4
  ;; Parameters: addr (i32), value (i32)
  ;; Behavior: Stores lower 16 bits of value at memory[addr + 4]
  ;; Effective address = addr + 4
  (func $store16_with_offset (param $addr i32) (param $value i32)
    (i32.store16 offset=4
      (local.get $addr)    ;; base address
      (local.get $value)   ;; i32 value to store (lower 16 bits used)
    )
  )
  (export "store16_with_offset" (func $store16_with_offset))

  ;; i32.store16 with large immediate offset for boundary testing
  ;; Parameters: addr (i32), value (i32)
  ;; Behavior: Stores lower 16 bits of value at memory[addr + 256]
  ;; Effective address = addr + 256
  (func $store16_large_offset (param $addr i32) (param $value i32)
    (i32.store16 offset=256
      (local.get $addr)    ;; base address
      (local.get $value)   ;; i32 value to store (lower 16 bits used)
    )
  )
  (export "store16_large_offset" (func $store16_large_offset))

  ;; Helper function: Load 16-bit unsigned value from memory
  ;; Parameters: addr (i32)
  ;; Returns: 16-bit value loaded from memory[addr] (zero-extended to i32)
  (func $load16_value (param $addr i32) (result i32)
    (i32.load16_u
      (local.get $addr)    ;; memory address
    )
  )
  (export "load16_value" (func $load16_value))

  ;; Helper function: Load 16-bit unsigned value with offset
  ;; Parameters: addr (i32)
  ;; Returns: 16-bit value loaded from memory[addr + 4] (zero-extended to i32)
  (func $load16_with_offset (param $addr i32) (result i32)
    (i32.load16_u offset=4
      (local.get $addr)    ;; base address
    )
  )
  (export "load16_with_offset" (func $load16_with_offset))

  ;; Helper function: Get current memory size in bytes
  ;; Returns: Memory size in bytes (pages * 65536)
  (func $get_memory_size (result i32)
    (i32.mul
      (memory.size)        ;; get memory size in pages
      (i32.const 65536)    ;; convert pages to bytes
    )
  )
  (export "get_memory_size" (func $get_memory_size))

  ;; Test function: Demonstrate truncation behavior
  ;; Parameters: addr (i32), large_value (i32)
  ;; Behavior: Stores large_value at addr, then loads to show truncation
  ;; Returns: The 16-bit value actually stored (for validation)
  (func $test_truncation (param $addr i32) (param $large_value i32) (result i32)
    ;; Store the large value (will be truncated to 16 bits)
    (i32.store16
      (local.get $addr)
      (local.get $large_value)
    )
    ;; Load and return the actual stored value (16-bit, zero-extended)
    (i32.load16_u
      (local.get $addr)
    )
  )
  (export "test_truncation" (func $test_truncation))

  ;; Test function: Multiple stores at different addresses
  ;; Parameters: base_addr (i32), val1 (i32), val2 (i32), val3 (i32)
  ;; Behavior: Stores three values at base_addr, base_addr+2, base_addr+4
  ;; Returns: none (validates multiple adjacent stores)
  (func $store_multiple (param $base_addr i32) (param $val1 i32) (param $val2 i32) (param $val3 i32)
    ;; Store first value at base address
    (i32.store16
      (local.get $base_addr)
      (local.get $val1)
    )
    ;; Store second value at base + 2
    (i32.store16
      (i32.add (local.get $base_addr) (i32.const 2))
      (local.get $val2)
    )
    ;; Store third value at base + 4
    (i32.store16
      (i32.add (local.get $base_addr) (i32.const 4))
      (local.get $val3)
    )
  )
  (export "store_multiple" (func $store_multiple))

  ;; Test function: Boundary store operations
  ;; Parameters: none
  ;; Behavior: Attempts stores at memory boundaries
  ;; Returns: 1 if successful, 0 if any operation failed
  (func $test_boundaries (result i32)
    (local $memory_size i32)
    (local $last_valid_addr i32)

    ;; Get current memory size
    (local.set $memory_size (call $get_memory_size))

    ;; Calculate last valid address for 16-bit store (memory_size - 2)
    (local.set $last_valid_addr
      (i32.sub (local.get $memory_size) (i32.const 2))
    )

    ;; Try to store at the last valid address
    (i32.store16
      (local.get $last_valid_addr)
      (i32.const 0xABCD)
    )

    ;; Verify the store was successful by reading back
    (i32.eq
      (i32.load16_u (local.get $last_valid_addr))
      (i32.const 0xABCD)
    )
  )
  (export "test_boundaries" (func $test_boundaries))

  ;; Test function: Zero value operations
  ;; Parameters: addr (i32)
  ;; Behavior: Stores and verifies zero values
  ;; Returns: 1 if zero storage worked correctly, 0 otherwise
  (func $test_zero_values (param $addr i32) (result i32)
    ;; Store zero value
    (i32.store16
      (local.get $addr)
      (i32.const 0)
    )

    ;; Verify zero was stored correctly
    (i32.eq
      (i32.load16_u (local.get $addr))
      (i32.const 0)
    )
  )
  (export "test_zero_values" (func $test_zero_values))

  ;; Test function: Maximum 16-bit values
  ;; Parameters: addr (i32)
  ;; Behavior: Stores and verifies maximum 16-bit values
  ;; Returns: 1 if maximum value storage worked correctly, 0 otherwise
  (func $test_max_values (param $addr i32) (result i32)
    ;; Store maximum 16-bit value (0xFFFF)
    (i32.store16
      (local.get $addr)
      (i32.const 0xFFFF)
    )

    ;; Verify maximum value was stored correctly
    (i32.eq
      (i32.load16_u (local.get $addr))
      (i32.const 0xFFFF)
    )
  )
  (export "test_max_values" (func $test_max_values))
)