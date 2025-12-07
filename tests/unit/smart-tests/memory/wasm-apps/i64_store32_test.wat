(module
  ;; Memory section with initial size of 2 pages (128KB)
  (memory (export "memory") 2)

  ;; Test function for i64.store32 opcode
  ;; Parameters: address (i32), value (i64), offset (i32)
  ;; Returns: status (i32) - 0 for success, 1 for failure
  (func (export "test_i64_store32") (param $address i32) (param $value i64) (param $offset i32) (result i32)
    (local $effective_addr i32)

    ;; Calculate effective address (address + offset)
    local.get $address
    local.get $offset
    i32.add
    local.set $effective_addr

    ;; Check if effective address is within bounds (memory size - 4 bytes for i32 store)
    local.get $effective_addr
    memory.size
    i32.const 65536  ;; Page size
    i32.mul
    i32.const 4      ;; Size of i32 store
    i32.sub
    i32.gt_u
    (if (result i32)
      (then
        ;; Out of bounds - return error status
        i32.const 1
      )
      (else
        ;; Perform i64.store32 operation
        local.get $effective_addr
        local.get $value
        i64.store32
        ;; Return success status
        i32.const 0
      )
    )
  )

  ;; Helper function to read i32 value from memory
  ;; Parameters: address (i32)
  ;; Returns: value (i32)
  (func (export "read_i32") (param $address i32) (result i32)
    local.get $address
    i32.load
  )

  ;; Helper function to get current memory size in bytes
  ;; Returns: memory size in bytes (i32)
  (func (export "get_memory_size") (result i32)
    memory.size
    i32.const 65536  ;; Page size in bytes
    i32.mul
  )

  ;; Test function for basic i64.store32 without bounds checking
  ;; Used for testing normal operation scenarios
  ;; Parameters: address (i32), value (i64)
  ;; Returns: nothing
  (func (export "store_i64_as_i32") (param $address i32) (param $value i64)
    local.get $address
    local.get $value
    i64.store32
  )

  ;; Test function for i64.store32 with immediate offset
  ;; Parameters: address (i32), value (i64)
  ;; Returns: nothing
  (func (export "store_i64_as_i32_offset4") (param $address i32) (param $value i64)
    local.get $address
    local.get $value
    i64.store32 offset=4
  )

  ;; Test function for i64.store32 with larger immediate offset
  ;; Parameters: address (i32), value (i64)
  ;; Returns: nothing
  (func (export "store_i64_as_i32_offset16") (param $address i32) (param $value i64)
    local.get $address
    local.get $value
    i64.store32 offset=16
  )

  ;; Test function for i64.store32 with alignment specification
  ;; Parameters: address (i32), value (i64)
  ;; Returns: nothing
  (func (export "store_i64_as_i32_aligned") (param $address i32) (param $value i64)
    local.get $address
    local.get $value
    i64.store32 align=4
  )

  ;; Test function for i64.store32 with both offset and alignment
  ;; Parameters: address (i32), value (i64)
  ;; Returns: nothing
  (func (export "store_i64_as_i32_offset_aligned") (param $address i32) (param $value i64)
    local.get $address
    local.get $value
    i64.store32 offset=8 align=4
  )

  ;; Test function for sequential i64.store32 operations
  ;; Parameters: base_address (i32), value1 (i64), value2 (i64), value3 (i64)
  ;; Returns: nothing
  (func (export "store_sequence") (param $base i32) (param $val1 i64) (param $val2 i64) (param $val3 i64)
    ;; Store first value at base address
    local.get $base
    local.get $val1
    i64.store32

    ;; Store second value at base + 4
    local.get $base
    i32.const 4
    i32.add
    local.get $val2
    i64.store32

    ;; Store third value at base + 8
    local.get $base
    i32.const 8
    i32.add
    local.get $val3
    i64.store32
  )

  ;; Test function for boundary condition testing
  ;; Attempts to store at the last valid 4-byte aligned position
  ;; Parameters: value (i64)
  ;; Returns: status (i32) - 0 for success, 1 for failure
  (func (export "test_boundary_store") (param $value i64) (result i32)
    (local $last_valid_addr i32)

    ;; Calculate last valid address for 4-byte store
    memory.size
    i32.const 65536  ;; Page size
    i32.mul
    i32.const 4      ;; Size of store operation
    i32.sub
    local.set $last_valid_addr

    ;; Attempt to store at last valid address
    local.get $last_valid_addr
    local.get $value
    i64.store32

    ;; Return success
    i32.const 0
  )

  ;; Test function for out-of-bounds store (should trap)
  ;; Parameters: value (i64)
  ;; Returns: nothing (should trap before returning)
  (func (export "test_out_of_bounds_store") (param $value i64)
    ;; Try to store beyond memory boundary
    memory.size
    i32.const 65536  ;; Page size
    i32.mul
    local.get $value
    i64.store32  ;; This should cause a trap
  )

  ;; Test function for address overflow scenarios
  ;; Parameters: high_address (i32), value (i64)
  ;; Returns: nothing (may trap depending on address)
  (func (export "test_address_overflow") (param $high_addr i32) (param $value i64)
    local.get $high_addr
    local.get $value
    i64.store32 offset=0xFFFFFFFF  ;; Large offset to test overflow
  )

  ;; Verification function to read and compare stored values
  ;; Parameters: address (i32), expected (i32)
  ;; Returns: comparison result (i32) - 1 if equal, 0 if different
  (func (export "verify_stored_value") (param $address i32) (param $expected i32) (result i32)
    local.get $address
    i32.load
    local.get $expected
    i32.eq
  )

  ;; Memory initialization function for test setup
  ;; Fills a memory region with a specific pattern for testing
  ;; Parameters: start (i32), size (i32), pattern (i32)
  (func (export "init_memory_pattern") (param $start i32) (param $size i32) (param $pattern i32)
    (local $i i32)
    (local $end i32)

    local.get $start
    local.get $size
    i32.add
    local.set $end

    local.get $start
    local.set $i

    (loop $fill_loop
      local.get $i
      local.get $end
      i32.lt_u
      (if
        (then
          local.get $i
          local.get $pattern
          i32.store

          local.get $i
          i32.const 4
          i32.add
          local.set $i

          br $fill_loop
        )
      )
    )
  )

  ;; Test function for unaligned access scenarios
  ;; Parameters: address (i32), value (i64)
  ;; Returns: nothing
  (func (export "test_unaligned_store") (param $address i32) (param $value i64)
    ;; Store at potentially unaligned address
    local.get $address
    local.get $value
    i64.store32 align=1  ;; Use minimal alignment
  )

  ;; Comprehensive test function combining multiple operations
  ;; Parameters: base (i32), test_value (i64)
  ;; Returns: test results bitmask (i32)
  (func (export "comprehensive_test") (param $base i32) (param $test_value i64) (result i32)
    (local $results i32)
    (local $temp_addr i32)

    i32.const 0
    local.set $results

    ;; Test 1: Basic store and verify
    local.get $base
    local.get $test_value
    i64.store32

    local.get $base
    i32.load
    local.get $test_value
    i32.wrap_i64
    i32.eq
    (if
      (then
        local.get $results
        i32.const 1
        i32.or
        local.set $results
      )
    )

    ;; Test 2: Store with offset and verify
    local.get $base
    i32.const 8
    i32.add
    local.set $temp_addr

    local.get $temp_addr
    local.get $test_value
    i64.store32 offset=4

    local.get $temp_addr
    i32.const 4
    i32.add
    i32.load
    local.get $test_value
    i32.wrap_i64
    i32.eq
    (if
      (then
        local.get $results
        i32.const 2
        i32.or
        local.set $results
      )
    )

    ;; Return combined results
    local.get $results
  )
)