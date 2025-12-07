(module
  ;; Memory with 1 page (64KB) for testing
  (memory (export "memory") 1)

  ;; Test function for v128.store64_lane with lane 0
  (func (export "test_store64_lane0") (param $addr i32) (param $low0 i32) (param $high0 i32) (param $low1 i32)
    ;; Build 64-bit value for lane 0: (high0 << 32) | low0
    local.get $addr
    local.get $low0
    i64.extend_i32_u
    local.get $high0
    i64.extend_i32_u
    i64.const 32
    i64.shl
    i64.or
    ;; Create v128 with this value in both lanes, then store lane 0
    i64x2.splat
    v128.store64_lane 0
  )

  ;; Test function for v128.store64_lane with lane 1
  (func (export "test_store64_lane1") (param $addr i32) (param $low0 i32) (param $high0 i32) (param $low1 i32)
    ;; Create v128 with lane 0 = (high0 << 32) | low0, lane 1 = 0x4444444433333333
    local.get $addr
    local.get $low0
    i64.extend_i32_u
    local.get $high0
    i64.extend_i32_u
    i64.const 32
    i64.shl
    i64.or
    i64x2.splat
    ;; Replace lane 1 with 0x4444444433333333
    i64.const 0x4444444433333333
    i64x2.replace_lane 1
    v128.store64_lane 1
  )

  ;; Test function for aligned memory access
  (func (export "test_store64_aligned") (param $addr i32) (param $low i32) (param $high i32) (param $unused i32)
    local.get $addr
    local.get $low
    i64.extend_i32_u
    local.get $high
    i64.extend_i32_u
    i64.const 32
    i64.shl
    i64.or
    i64x2.splat
    v128.store64_lane 0
  )

  ;; Test function for unaligned memory access
  (func (export "test_store64_unaligned") (param $addr i32) (param $low i32) (param $high i32) (param $unused i32)
    local.get $addr
    local.get $low
    i64.extend_i32_u
    local.get $high
    i64.extend_i32_u
    i64.const 32
    i64.shl
    i64.or
    i64x2.splat
    v128.store64_lane 0
  )

  ;; Test function for basic offset (used by C++ test)
  (func (export "test_store64_offset") (param $base i32) (param $low i32) (param $high i32) (param $offset i32)
    local.get $base
    local.get $low
    i64.extend_i32_u
    local.get $high
    i64.extend_i32_u
    i64.const 32
    i64.shl
    i64.or
    i64x2.splat
    v128.store64_lane 0
  )

  ;; Test function for special bit patterns
  (func (export "test_store64_pattern") (param $addr i32) (param $low i32) (param $high i32) (param $unused i32)
    local.get $addr
    local.get $low
    i64.extend_i32_u
    local.get $high
    i64.extend_i32_u
    i64.const 32
    i64.shl
    i64.or
    i64x2.splat
    v128.store64_lane 0
  )

  ;; Test function for bounds checking
  (func (export "test_store64_bounds") (param $addr i32) (param $low i32) (param $high i32) (param $unused i32)
    local.get $addr
    local.get $low
    i64.extend_i32_u
    local.get $high
    i64.extend_i32_u
    i64.const 32
    i64.shl
    i64.or
    i64x2.splat
    v128.store64_lane 0
  )
)