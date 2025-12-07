;; WASM test module for v128.store16_lane opcode validation
;;
;; This module provides test functions for the v128.store16_lane instruction,
;; which stores a 16-bit value from a specific lane of a v128 SIMD vector
;; to linear memory at a calculated address.

(module
  ;; Define linear memory: 1 page (64KB) for comprehensive testing
  (memory (export "memory") 1)

  ;; Store from lane 0: Tests lane index 0 with various memory scenarios
  (func (export "store_lane_0") (param $offset i32) (param $lane0 i32) (param $lane1 i32) (param $lane2 i32) (param $lane3 i32)
                                (param $lane4 i32) (param $lane5 i32) (param $lane6 i32) (param $lane7 i32)
    ;; Push address and v128 vector to stack
    local.get $offset
    ;; Create v128 vector with provided lane values
    i32.const 0
    i16x8.splat
    local.get $lane0
    i16x8.replace_lane 0
    local.get $lane1
    i16x8.replace_lane 1
    local.get $lane2
    i16x8.replace_lane 2
    local.get $lane3
    i16x8.replace_lane 3
    local.get $lane4
    i16x8.replace_lane 4
    local.get $lane5
    i16x8.replace_lane 5
    local.get $lane6
    i16x8.replace_lane 6
    local.get $lane7
    i16x8.replace_lane 7
    ;; Store 16-bit value from lane 0 to memory at offset
    v128.store16_lane 0
  )

  ;; Store from lane 1: Tests lane index 1 extraction and storage
  (func (export "store_lane_1") (param $offset i32) (param $lane0 i32) (param $lane1 i32) (param $lane2 i32) (param $lane3 i32)
                                (param $lane4 i32) (param $lane5 i32) (param $lane6 i32) (param $lane7 i32)
    local.get $offset
    i32.const 0
    i16x8.splat
    local.get $lane0
    i16x8.replace_lane 0
    local.get $lane1
    i16x8.replace_lane 1
    local.get $lane2
    i16x8.replace_lane 2
    local.get $lane3
    i16x8.replace_lane 3
    local.get $lane4
    i16x8.replace_lane 4
    local.get $lane5
    i16x8.replace_lane 5
    local.get $lane6
    i16x8.replace_lane 6
    local.get $lane7
    i16x8.replace_lane 7
    v128.store16_lane 1
  )

  ;; Store from lane 2: Tests middle lane access patterns
  (func (export "store_lane_2") (param $offset i32) (param $lane0 i32) (param $lane1 i32) (param $lane2 i32) (param $lane3 i32)
                                (param $lane4 i32) (param $lane5 i32) (param $lane6 i32) (param $lane7 i32)
    local.get $offset
    i32.const 0
    i16x8.splat
    local.get $lane0
    i16x8.replace_lane 0
    local.get $lane1
    i16x8.replace_lane 1
    local.get $lane2
    i16x8.replace_lane 2
    local.get $lane3
    i16x8.replace_lane 3
    local.get $lane4
    i16x8.replace_lane 4
    local.get $lane5
    i16x8.replace_lane 5
    local.get $lane6
    i16x8.replace_lane 6
    local.get $lane7
    i16x8.replace_lane 7
    v128.store16_lane 2
  )

  ;; Store from lane 3: Tests lane index 3 with address calculations
  (func (export "store_lane_3") (param $offset i32) (param $lane0 i32) (param $lane1 i32) (param $lane2 i32) (param $lane3 i32)
                                (param $lane4 i32) (param $lane5 i32) (param $lane6 i32) (param $lane7 i32)
    local.get $offset
    i32.const 0
    i16x8.splat
    local.get $lane0
    i16x8.replace_lane 0
    local.get $lane1
    i16x8.replace_lane 1
    local.get $lane2
    i16x8.replace_lane 2
    local.get $lane3
    i16x8.replace_lane 3
    local.get $lane4
    i16x8.replace_lane 4
    local.get $lane5
    i16x8.replace_lane 5
    local.get $lane6
    i16x8.replace_lane 6
    local.get $lane7
    i16x8.replace_lane 7
    v128.store16_lane 3
  )

  ;; Store from lane 4: Tests second half of vector lanes
  (func (export "store_lane_4") (param $offset i32) (param $lane0 i32) (param $lane1 i32) (param $lane2 i32) (param $lane3 i32)
                                (param $lane4 i32) (param $lane5 i32) (param $lane6 i32) (param $lane7 i32)
    local.get $offset
    i32.const 0
    i16x8.splat
    local.get $lane0
    i16x8.replace_lane 0
    local.get $lane1
    i16x8.replace_lane 1
    local.get $lane2
    i16x8.replace_lane 2
    local.get $lane3
    i16x8.replace_lane 3
    local.get $lane4
    i16x8.replace_lane 4
    local.get $lane5
    i16x8.replace_lane 5
    local.get $lane6
    i16x8.replace_lane 6
    local.get $lane7
    i16x8.replace_lane 7
    v128.store16_lane 4
  )

  ;; Store from lane 5: Tests upper middle lane extraction
  (func (export "store_lane_5") (param $offset i32) (param $lane0 i32) (param $lane1 i32) (param $lane2 i32) (param $lane3 i32)
                                (param $lane4 i32) (param $lane5 i32) (param $lane6 i32) (param $lane7 i32)
    local.get $offset
    i32.const 0
    i16x8.splat
    local.get $lane0
    i16x8.replace_lane 0
    local.get $lane1
    i16x8.replace_lane 1
    local.get $lane2
    i16x8.replace_lane 2
    local.get $lane3
    i16x8.replace_lane 3
    local.get $lane4
    i16x8.replace_lane 4
    local.get $lane5
    i16x8.replace_lane 5
    local.get $lane6
    i16x8.replace_lane 6
    local.get $lane7
    i16x8.replace_lane 7
    v128.store16_lane 5
  )

  ;; Store from lane 6: Tests near-end lane access patterns
  (func (export "store_lane_6") (param $offset i32) (param $lane0 i32) (param $lane1 i32) (param $lane2 i32) (param $lane3 i32)
                                (param $lane4 i32) (param $lane5 i32) (param $lane6 i32) (param $lane7 i32)
    local.get $offset
    i32.const 0
    i16x8.splat
    local.get $lane0
    i16x8.replace_lane 0
    local.get $lane1
    i16x8.replace_lane 1
    local.get $lane2
    i16x8.replace_lane 2
    local.get $lane3
    i16x8.replace_lane 3
    local.get $lane4
    i16x8.replace_lane 4
    local.get $lane5
    i16x8.replace_lane 5
    local.get $lane6
    i16x8.replace_lane 6
    local.get $lane7
    i16x8.replace_lane 7
    v128.store16_lane 6
  )

  ;; Store from lane 7: Tests maximum lane index (boundary condition)
  (func (export "store_lane_7") (param $offset i32) (param $lane0 i32) (param $lane1 i32) (param $lane2 i32) (param $lane3 i32)
                                (param $lane4 i32) (param $lane5 i32) (param $lane6 i32) (param $lane7 i32)
    local.get $offset
    i32.const 0
    i16x8.splat
    local.get $lane0
    i16x8.replace_lane 0
    local.get $lane1
    i16x8.replace_lane 1
    local.get $lane2
    i16x8.replace_lane 2
    local.get $lane3
    i16x8.replace_lane 3
    local.get $lane4
    i16x8.replace_lane 4
    local.get $lane5
    i16x8.replace_lane 5
    local.get $lane6
    i16x8.replace_lane 6
    local.get $lane7
    i16x8.replace_lane 7
    v128.store16_lane 7
  )

  ;; Utility function to read memory content for verification
  ;; Returns the 16-bit value stored at specified address
  (func (export "read_memory_u16") (param $addr i32) (result i32)
    (i32.load16_u (local.get $addr))
  )
)