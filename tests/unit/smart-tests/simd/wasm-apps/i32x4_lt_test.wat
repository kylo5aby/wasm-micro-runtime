(module
  ;; Import WASM SIMD feature check
  (import "spectest" "print_i32" (func $print_i32 (param i32)))

  ;; Test function for basic i32x4.lt operations
  (func (export "test_i32x4_lt_basic") (param v128) (param v128) (result v128)
    local.get 0
    local.get 1
    i32x4.lt_u  ;; Unsigned less-than comparison
  )

  ;; Test function for boundary value i32x4.lt operations
  (func (export "test_i32x4_lt_boundary") (param v128) (param v128) (result v128)
    local.get 0
    local.get 1
    i32x4.lt_u  ;; Unsigned less-than comparison
  )

  ;; Test function for equal values i32x4.lt operations
  (func (export "test_i32x4_lt_equal") (param v128) (param v128) (result v128)
    local.get 0
    local.get 1
    i32x4.lt_u  ;; Unsigned less-than comparison
  )

  ;; Test function for unsigned semantics validation
  (func (export "test_i32x4_lt_unsigned") (param v128) (param v128) (result v128)
    local.get 0
    local.get 1
    i32x4.lt_u  ;; Unsigned less-than comparison
  )

  ;; Test function for mixed result patterns
  (func (export "test_i32x4_lt_mixed") (param v128) (param v128) (result v128)
    local.get 0
    local.get 1
    i32x4.lt_u  ;; Unsigned less-than comparison
  )

  ;; Helper function to create v128 from four i32 values (for testing)
  (func (export "create_v128") (param i32) (param i32) (param i32) (param i32) (result v128)
    local.get 0
    i32x4.splat
    local.get 1
    i32x4.replace_lane 1
    local.get 2
    i32x4.replace_lane 2
    local.get 3
    i32x4.replace_lane 3
  )

  ;; Helper function to extract i32 lanes from v128 (for debugging)
  (func (export "extract_lane_0") (param v128) (result i32)
    local.get 0
    i32x4.extract_lane 0
  )

  (func (export "extract_lane_1") (param v128) (result i32)
    local.get 0
    i32x4.extract_lane 1
  )

  (func (export "extract_lane_2") (param v128) (result i32)
    local.get 0
    i32x4.extract_lane 2
  )

  (func (export "extract_lane_3") (param v128) (result i32)
    local.get 0
    i32x4.extract_lane 3
  )

  ;; Test specific unsigned comparison cases
  (func (export "test_unsigned_boundary_cases") (result v128)
    ;; Test: [0, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFE] < [0x80000000, 0x80000000, 0xFFFFFFFF, 0xFFFFFFFF]
    ;; Expected: [true, true, true, true] (all lanes should be true in unsigned comparison)

    ;; Create first vector: [0, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFE]
    i32.const 0
    i32x4.splat
    i32.const 0x7FFFFFFF
    i32x4.replace_lane 1
    i32.const 0x80000000
    i32x4.replace_lane 2
    i32.const 0xFFFFFFFE
    i32x4.replace_lane 3

    ;; Create second vector: [0x80000000, 0x80000000, 0xFFFFFFFF, 0xFFFFFFFF]
    i32.const 0x80000000
    i32x4.splat
    i32.const 0xFFFFFFFF
    i32x4.replace_lane 2
    i32.const 0xFFFFFFFF
    i32x4.replace_lane 3

    ;; Perform comparison
    i32x4.lt_u
  )

  ;; Test signed vs unsigned distinction
  (func (export "test_signed_vs_unsigned") (result v128)
    ;; Test: [0x80000000, 0xFFFFFFFF] < [0x7FFFFFFF, 0x80000000]
    ;; In unsigned: [false, false] (0x80000000 > 0x7FFFFFFF, 0xFFFFFFFF > 0x80000000)
    ;; In signed: [true, false] (INT32_MIN < INT32_MAX, -1 > INT32_MIN)

    ;; Create first vector: [0x80000000, 0xFFFFFFFF, 0, 0]
    i32.const 0x80000000
    i32x4.splat
    i32.const 0xFFFFFFFF
    i32x4.replace_lane 1
    i32.const 0
    i32x4.replace_lane 2
    i32.const 0
    i32x4.replace_lane 3

    ;; Create second vector: [0x7FFFFFFF, 0x80000000, 0, 0]
    i32.const 0x7FFFFFFF
    i32x4.splat
    i32.const 0x80000000
    i32x4.replace_lane 1
    i32.const 0
    i32x4.replace_lane 2
    i32.const 0
    i32x4.replace_lane 3

    ;; Perform unsigned comparison
    i32x4.lt_u
  )

  ;; Memory for testing (if needed)
  (memory 1)
)