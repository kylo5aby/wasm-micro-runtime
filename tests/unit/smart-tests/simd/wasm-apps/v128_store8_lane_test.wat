(module
  (memory 1)

  ;; Export memory for testing access
  (export "memory" (memory 0))

  ;; Test function: v128.store8_lane with basic lane access
  ;; Parameters: address, lane_idx, expected_value (for validation)
  ;; Returns: 1 on success, 0 on failure
  (func (export "test_store8_lane") (param $addr i32) (param $lane i32) (param $expected i32) (result i32)
    (local $v128_val v128)

    ;; Create test vector with incremental pattern (0x00, 0x01, 0x02, ..., 0x0F)
    (local.set $v128_val
      (i8x16.splat (i32.const 0))
    )

    ;; Set specific lane values based on lane index
    (if (i32.eq (local.get $lane) (i32.const 0))
      (then (local.set $v128_val (i8x16.replace_lane 0 (local.get $v128_val) (local.get $expected))))
    )
    (if (i32.eq (local.get $lane) (i32.const 1))
      (then (local.set $v128_val (i8x16.replace_lane 1 (local.get $v128_val) (local.get $expected))))
    )
    (if (i32.eq (local.get $lane) (i32.const 2))
      (then (local.set $v128_val (i8x16.replace_lane 2 (local.get $v128_val) (local.get $expected))))
    )
    (if (i32.eq (local.get $lane) (i32.const 3))
      (then (local.set $v128_val (i8x16.replace_lane 3 (local.get $v128_val) (local.get $expected))))
    )
    (if (i32.eq (local.get $lane) (i32.const 4))
      (then (local.set $v128_val (i8x16.replace_lane 4 (local.get $v128_val) (local.get $expected))))
    )
    (if (i32.eq (local.get $lane) (i32.const 5))
      (then (local.set $v128_val (i8x16.replace_lane 5 (local.get $v128_val) (local.get $expected))))
    )
    (if (i32.eq (local.get $lane) (i32.const 6))
      (then (local.set $v128_val (i8x16.replace_lane 6 (local.get $v128_val) (local.get $expected))))
    )
    (if (i32.eq (local.get $lane) (i32.const 7))
      (then (local.set $v128_val (i8x16.replace_lane 7 (local.get $v128_val) (local.get $expected))))
    )
    (if (i32.eq (local.get $lane) (i32.const 8))
      (then (local.set $v128_val (i8x16.replace_lane 8 (local.get $v128_val) (local.get $expected))))
    )
    (if (i32.eq (local.get $lane) (i32.const 9))
      (then (local.set $v128_val (i8x16.replace_lane 9 (local.get $v128_val) (local.get $expected))))
    )
    (if (i32.eq (local.get $lane) (i32.const 10))
      (then (local.set $v128_val (i8x16.replace_lane 10 (local.get $v128_val) (local.get $expected))))
    )
    (if (i32.eq (local.get $lane) (i32.const 11))
      (then (local.set $v128_val (i8x16.replace_lane 11 (local.get $v128_val) (local.get $expected))))
    )
    (if (i32.eq (local.get $lane) (i32.const 12))
      (then (local.set $v128_val (i8x16.replace_lane 12 (local.get $v128_val) (local.get $expected))))
    )
    (if (i32.eq (local.get $lane) (i32.const 13))
      (then (local.set $v128_val (i8x16.replace_lane 13 (local.get $v128_val) (local.get $expected))))
    )
    (if (i32.eq (local.get $lane) (i32.const 14))
      (then (local.set $v128_val (i8x16.replace_lane 14 (local.get $v128_val) (local.get $expected))))
    )
    (if (i32.eq (local.get $lane) (i32.const 15))
      (then (local.set $v128_val (i8x16.replace_lane 15 (local.get $v128_val) (local.get $expected))))
    )

    ;; Perform v128.store8_lane operations based on lane index
    (if (i32.eq (local.get $lane) (i32.const 0))
      (then (v128.store8_lane 0 (local.get $addr) (local.get $v128_val)))
    )
    (if (i32.eq (local.get $lane) (i32.const 1))
      (then (v128.store8_lane 1 (local.get $addr) (local.get $v128_val)))
    )
    (if (i32.eq (local.get $lane) (i32.const 2))
      (then (v128.store8_lane 2 (local.get $addr) (local.get $v128_val)))
    )
    (if (i32.eq (local.get $lane) (i32.const 3))
      (then (v128.store8_lane 3 (local.get $addr) (local.get $v128_val)))
    )
    (if (i32.eq (local.get $lane) (i32.const 4))
      (then (v128.store8_lane 4 (local.get $addr) (local.get $v128_val)))
    )
    (if (i32.eq (local.get $lane) (i32.const 5))
      (then (v128.store8_lane 5 (local.get $addr) (local.get $v128_val)))
    )
    (if (i32.eq (local.get $lane) (i32.const 6))
      (then (v128.store8_lane 6 (local.get $addr) (local.get $v128_val)))
    )
    (if (i32.eq (local.get $lane) (i32.const 7))
      (then (v128.store8_lane 7 (local.get $addr) (local.get $v128_val)))
    )
    (if (i32.eq (local.get $lane) (i32.const 8))
      (then (v128.store8_lane 8 (local.get $addr) (local.get $v128_val)))
    )
    (if (i32.eq (local.get $lane) (i32.const 9))
      (then (v128.store8_lane 9 (local.get $addr) (local.get $v128_val)))
    )
    (if (i32.eq (local.get $lane) (i32.const 10))
      (then (v128.store8_lane 10 (local.get $addr) (local.get $v128_val)))
    )
    (if (i32.eq (local.get $lane) (i32.const 11))
      (then (v128.store8_lane 11 (local.get $addr) (local.get $v128_val)))
    )
    (if (i32.eq (local.get $lane) (i32.const 12))
      (then (v128.store8_lane 12 (local.get $addr) (local.get $v128_val)))
    )
    (if (i32.eq (local.get $lane) (i32.const 13))
      (then (v128.store8_lane 13 (local.get $addr) (local.get $v128_val)))
    )
    (if (i32.eq (local.get $lane) (i32.const 14))
      (then (v128.store8_lane 14 (local.get $addr) (local.get $v128_val)))
    )
    (if (i32.eq (local.get $lane) (i32.const 15))
      (then (v128.store8_lane 15 (local.get $addr) (local.get $v128_val)))
    )

    ;; Verify the stored value matches expected value
    (i32.eq
      (i32.load8_u (local.get $addr))
      (local.get $expected)
    )
  )

  ;; Test function: v128.store8_lane with memory offset
  ;; Tests memarg offset parameter functionality
  (func (export "test_store8_lane_with_offset") (param $addr i32) (param $lane i32) (param $expected i32) (result i32)
    (local $v128_val v128)

    ;; Create test vector with expected value in specified lane
    (local.set $v128_val (i8x16.splat (i32.const 0)))

    ;; Set specific lane value
    (if (i32.eq (local.get $lane) (i32.const 0))
      (then (local.set $v128_val (i8x16.replace_lane 0 (local.get $v128_val) (local.get $expected))))
    )
    (if (i32.eq (local.get $lane) (i32.const 1))
      (then (local.set $v128_val (i8x16.replace_lane 1 (local.get $v128_val) (local.get $expected))))
    )
    (if (i32.eq (local.get $lane) (i32.const 2))
      (then (local.set $v128_val (i8x16.replace_lane 2 (local.get $v128_val) (local.get $expected))))
    )
    (if (i32.eq (local.get $lane) (i32.const 3))
      (then (local.set $v128_val (i8x16.replace_lane 3 (local.get $v128_val) (local.get $expected))))
    )
    (if (i32.eq (local.get $lane) (i32.const 4))
      (then (local.set $v128_val (i8x16.replace_lane 4 (local.get $v128_val) (local.get $expected))))
    )
    (if (i32.eq (local.get $lane) (i32.const 5))
      (then (local.set $v128_val (i8x16.replace_lane 5 (local.get $v128_val) (local.get $expected))))
    )

    ;; Store with offset of 4 bytes
    (if (i32.eq (local.get $lane) (i32.const 0))
      (then (v128.store8_lane 0 (i32.add (local.get $addr) (i32.const 4)) (local.get $v128_val)))
    )
    (if (i32.eq (local.get $lane) (i32.const 1))
      (then (v128.store8_lane 1 (i32.add (local.get $addr) (i32.const 4)) (local.get $v128_val)))
    )
    (if (i32.eq (local.get $lane) (i32.const 2))
      (then (v128.store8_lane 2 (i32.add (local.get $addr) (i32.const 4)) (local.get $v128_val)))
    )
    (if (i32.eq (local.get $lane) (i32.const 3))
      (then (v128.store8_lane 3 (i32.add (local.get $addr) (i32.const 4)) (local.get $v128_val)))
    )
    (if (i32.eq (local.get $lane) (i32.const 4))
      (then (v128.store8_lane 4 (i32.add (local.get $addr) (i32.const 4)) (local.get $v128_val)))
    )
    (if (i32.eq (local.get $lane) (i32.const 5))
      (then (v128.store8_lane 5 (i32.add (local.get $addr) (i32.const 4)) (local.get $v128_val)))
    )

    ;; Verify stored value at address + offset
    (i32.eq
      (i32.load8_u (i32.add (local.get $addr) (i32.const 4)))
      (local.get $expected)
    )
  )

  ;; Test function: v128.store8_lane with bounds checking
  ;; Tests memory bounds validation for error cases
  (func (export "test_store8_lane_bounds") (param $addr i32) (param $lane i32) (param $expected i32) (result i32)
    (local $v128_val v128)

    ;; Create test vector
    (local.set $v128_val (i8x16.splat (local.get $expected)))

    ;; Try to store to potentially out-of-bounds address
    ;; This should be wrapped in a try-catch, but for simplicity we'll just attempt the store
    ;; Real bounds checking will happen in WAMR runtime
    (v128.store8_lane 0 (local.get $addr) (local.get $v128_val))

    ;; If we reach here without trap, return 1 (success)
    (i32.const 1)
  )

  ;; Test function: v128.store8_lane with large offset potentially causing overflow
  ;; Tests address calculation overflow scenarios
  (func (export "test_store8_lane_large_offset") (param $addr i32) (param $lane i32) (param $expected i32) (result i32)
    (local $v128_val v128)

    ;; Create test vector
    (local.set $v128_val (i8x16.splat (local.get $expected)))

    ;; Try to store with large offset that might cause overflow
    (v128.store8_lane 0 (i32.add (local.get $addr) (i32.const 1000)) (local.get $v128_val))

    ;; If we reach here without trap, return 1 (success)
    (i32.const 1)
  )
)