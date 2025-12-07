;; Enhanced WebAssembly Test Module for memory.fill Opcode
;;
;; This module provides comprehensive test functions for validating memory.fill
;; operations across different scenarios including basic fills, boundary
;; conditions, value truncation, zero-length operations, and error cases.
;;
;; Memory Layout:
;; - Memory size: 1 page (64KB) initial with expansion capability for testing
;; - Test regions distributed across memory to avoid interference
;; - Pattern-based data initialization for reliable validation
;;
;; Test Functions:
;; - memory_fill_test: Basic memory.fill wrapper function
;; - memory_fill_boundary: Boundary condition testing
;; - memory_fill_large: Large region fill testing
;; - get_memory_byte: Helper for memory content validation
;; - set_memory_byte: Helper for memory content setup

(module
  ;; Memory definition: 1 page (64KB) initial, 2 pages (128KB) maximum
  (memory $mem 1 2)

  ;; Export memory for external access and validation
  (export "memory" (memory $mem))

  ;; Basic memory.fill test function
  ;; Parameters: dest (i32), val (i32), size (i32)
  ;; Returns: none (operation success indicated by no trap)
  (func $memory_fill_test (param $dest i32) (param $val i32) (param $size i32)
    (memory.fill
      (local.get $dest)    ;; destination offset
      (local.get $val)     ;; fill value (truncated to byte)
      (local.get $size)    ;; number of bytes to fill
    )
  )
  (export "memory_fill_test" (func $memory_fill_test))

  ;; Memory fill with validation - returns 1 on success, 0 on failure
  ;; Parameters: dest (i32), val (i32), size (i32)
  ;; Returns: success flag (i32)
  (func $memory_fill_with_validation (param $dest i32) (param $val i32) (param $size i32) (result i32)
    (local $memory_size i32)

    ;; Get current memory size in bytes
    (local.set $memory_size
      (i32.mul
        (memory.size)
        (i32.const 65536)  ;; 64KB per page
      )
    )

    ;; Check if fill operation would exceed memory bounds
    (if (i32.gt_u
          (i32.add (local.get $dest) (local.get $size))
          (local.get $memory_size)
        )
      (then
        ;; Return 0 for out-of-bounds operation
        (return (i32.const 0))
      )
    )

    ;; Perform memory.fill operation
    (memory.fill
      (local.get $dest)
      (local.get $val)
      (local.get $size)
    )

    ;; Return success flag
    (i32.const 1)
  )
  (export "memory_fill_with_validation" (func $memory_fill_with_validation))

  ;; Boundary condition testing function
  ;; Tests fill operations at memory boundaries
  ;; Parameters: none
  ;; Returns: test result (i32)
  (func $test_boundary_fill (result i32)
    (local $memory_size i32)
    (local $last_offset i32)

    ;; Get memory size in bytes
    (local.set $memory_size
      (i32.mul (memory.size) (i32.const 65536))
    )

    ;; Calculate last valid offset
    (local.set $last_offset
      (i32.sub (local.get $memory_size) (i32.const 1))
    )

    ;; Fill single byte at last valid position
    (memory.fill
      (local.get $last_offset)
      (i32.const 0xFF)
      (i32.const 1)
    )

    ;; Verify the fill by reading the byte
    (if (result i32) (i32.eq
          (i32.load8_u (local.get $last_offset))
          (i32.const 0xFF)
        )
      (then (i32.const 1))  ;; Success
      (else (i32.const 0))  ;; Failure
    )
  )
  (export "test_boundary_fill" (func $test_boundary_fill))

  ;; Large region fill testing function
  ;; Tests fill operations on substantial memory regions
  ;; Parameters: dest (i32), val (i32), size (i32)
  ;; Returns: validation result (i32)
  (func $test_large_fill (param $dest i32) (param $val i32) (param $size i32) (result i32)
    (local $i i32)
    (local $check_step i32)

    ;; Perform the fill operation
    (memory.fill
      (local.get $dest)
      (local.get $val)
      (local.get $size)
    )

    ;; Validate fill by checking every 1000th byte
    (local.set $check_step (i32.const 1000))
    (local.set $i (i32.const 0))

    (loop $validation_loop
      ;; Check if current position is within range
      (if (i32.lt_u (local.get $i) (local.get $size))
        (then
          ;; Verify byte value at position
          (if (i32.ne
                (i32.load8_u (i32.add (local.get $dest) (local.get $i)))
                (i32.and (local.get $val) (i32.const 0xFF))
              )
            (then (return (i32.const 0)))  ;; Validation failed
          )

          ;; Increment to next check position
          (local.set $i
            (i32.add (local.get $i) (local.get $check_step))
          )

          ;; Continue loop
          (br $validation_loop)
        )
      )
    )

    ;; All checks passed
    (i32.const 1)
  )
  (export "test_large_fill" (func $test_large_fill))

  ;; Zero-length fill testing function
  ;; Tests that zero-length fills are no-ops
  ;; Parameters: dest (i32), val (i32)
  ;; Returns: validation result (i32)
  (func $test_zero_length_fill (param $dest i32) (param $val i32) (result i32)
    (local $original_value i32)

    ;; Store original byte value at destination
    (local.set $original_value
      (i32.load8_u (local.get $dest))
    )

    ;; Perform zero-length fill
    (memory.fill
      (local.get $dest)
      (local.get $val)
      (i32.const 0)
    )

    ;; Verify memory unchanged
    (if (result i32) (i32.eq
          (i32.load8_u (local.get $dest))
          (local.get $original_value)
        )
      (then (i32.const 1))  ;; Success - no change
      (else (i32.const 0))  ;; Failure - unexpected change
    )
  )
  (export "test_zero_length_fill" (func $test_zero_length_fill))

  ;; Value truncation testing function
  ;; Tests that high bits of value parameter are ignored
  ;; Parameters: dest (i32), full_value (i32), size (i32)
  ;; Returns: validation result (i32)
  (func $test_value_truncation (param $dest i32) (param $full_value i32) (param $size i32) (result i32)
    (local $expected_byte i32)
    (local $i i32)

    ;; Calculate expected byte value (low 8 bits only)
    (local.set $expected_byte
      (i32.and (local.get $full_value) (i32.const 0xFF))
    )

    ;; Perform fill with full 32-bit value
    (memory.fill
      (local.get $dest)
      (local.get $full_value)
      (local.get $size)
    )

    ;; Validate that all bytes match expected truncated value
    (local.set $i (i32.const 0))
    (loop $verify_loop
      (if (i32.lt_u (local.get $i) (local.get $size))
        (then
          ;; Check byte value
          (if (i32.ne
                (i32.load8_u (i32.add (local.get $dest) (local.get $i)))
                (local.get $expected_byte)
              )
            (then (return (i32.const 0)))  ;; Validation failed
          )

          ;; Move to next byte
          (local.set $i (i32.add (local.get $i) (i32.const 1)))
          (br $verify_loop)
        )
      )
    )

    ;; All bytes correctly truncated
    (i32.const 1)
  )
  (export "test_value_truncation" (func $test_value_truncation))

  ;; Helper function to read byte from memory
  ;; Parameters: offset (i32)
  ;; Returns: byte value (i32)
  (func $get_memory_byte (param $offset i32) (result i32)
    (i32.load8_u (local.get $offset))
  )
  (export "get_memory_byte" (func $get_memory_byte))

  ;; Helper function to write byte to memory
  ;; Parameters: offset (i32), value (i32)
  ;; Returns: none
  (func $set_memory_byte (param $offset i32) (param $value i32)
    (i32.store8
      (local.get $offset)
      (local.get $value)
    )
  )
  (export "set_memory_byte" (func $set_memory_byte))

  ;; Setup function for testing - initializes memory with distinct patterns
  ;; Parameters: start_offset (i32), length (i32)
  ;; Returns: none
  (func $setup_test_pattern (param $start_offset i32) (param $length i32)
    (local $i i32)
    (local $pattern_value i32)

    (local.set $i (i32.const 0))
    (loop $setup_loop
      (if (i32.lt_u (local.get $i) (local.get $length))
        (then
          ;; Calculate pattern value (i % 256)
          (local.set $pattern_value
            (i32.and (local.get $i) (i32.const 0xFF))
          )

          ;; Store pattern byte
          (i32.store8
            (i32.add (local.get $start_offset) (local.get $i))
            (local.get $pattern_value)
          )

          ;; Next iteration
          (local.set $i (i32.add (local.get $i) (i32.const 1)))
          (br $setup_loop)
        )
      )
    )
  )
  (export "setup_test_pattern" (func $setup_test_pattern))

  ;; Memory grow testing function
  ;; Tests memory.fill after memory expansion
  ;; Parameters: none
  ;; Returns: test result (i32)
  (func $test_memory_grow_fill (result i32)
    (local $old_size i32)
    (local $new_size i32)
    (local $fill_offset i32)

    ;; Get initial memory size
    (local.set $old_size (memory.size))

    ;; Grow memory by 1 page
    (drop (memory.grow (i32.const 1)))

    ;; Get new memory size
    (local.set $new_size (memory.size))

    ;; Verify memory grew
    (if (i32.le_u (local.get $new_size) (local.get $old_size))
      (then (return (i32.const 0)))  ;; Memory didn't grow
    )

    ;; Calculate offset in new memory region
    (local.set $fill_offset
      (i32.mul (local.get $old_size) (i32.const 65536))
    )

    ;; Fill new memory region
    (memory.fill
      (local.get $fill_offset)
      (i32.const 0x42)
      (i32.const 1024)
    )

    ;; Verify fill success by checking first and last bytes
    (if (result i32) (i32.and
          (i32.eq
            (i32.load8_u (local.get $fill_offset))
            (i32.const 0x42)
          )
          (i32.eq
            (i32.load8_u (i32.add (local.get $fill_offset) (i32.const 1023)))
            (i32.const 0x42)
          )
        )
      (then (i32.const 1))  ;; Success
      (else (i32.const 0))  ;; Failure
    )
  )
  (export "test_memory_grow_fill" (func $test_memory_grow_fill))

  ;; Helper function to get current memory size in bytes
  ;; Returns current memory size (i32)
  (func $get_memory_size (result i32)
    (i32.mul
      (memory.size)
      (i32.const 65536)  ;; 64KB per page
    )
  )
  (export "get_memory_size" (func $get_memory_size))
)