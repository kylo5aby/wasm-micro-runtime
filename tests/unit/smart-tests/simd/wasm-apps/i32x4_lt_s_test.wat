;; Enhanced test module for i32x4.lt_s SIMD opcode
;; Tests signed less-than comparison on 4 x 32-bit integer lanes

(module
  ;; Test function for basic i32x4.lt_s functionality
  ;; Takes two v128 vectors, performs i32x4.lt_s, returns result
  (func (export "test_i32x4_lt_s_basic") (param $a0 i32) (param $a1 i32) (param $a2 i32) (param $a3 i32)
                                          (param $b0 i32) (param $b1 i32) (param $b2 i32) (param $b3 i32)
                                          (result i32 i32 i32 i32)
    ;; Create first vector from individual i32 parameters
    (local $vec_a v128)
    (local $vec_b v128)
    (local $result v128)

    ;; Build vector A: [a0, a1, a2, a3]
    local.get $a0
    i32x4.splat
    local.get $a1
    i32x4.replace_lane 1
    local.get $a2
    i32x4.replace_lane 2
    local.get $a3
    i32x4.replace_lane 3
    local.set $vec_a

    ;; Build vector B: [b0, b1, b2, b3]
    local.get $b0
    i32x4.splat
    local.get $b1
    i32x4.replace_lane 1
    local.get $b2
    i32x4.replace_lane 2
    local.get $b3
    i32x4.replace_lane 3
    local.set $vec_b

    ;; Perform i32x4.lt_s comparison
    local.get $vec_a
    local.get $vec_b
    i32x4.lt_s
    local.set $result

    ;; Extract result lanes and return
    local.get $result
    i32x4.extract_lane 0
    local.get $result
    i32x4.extract_lane 1
    local.get $result
    i32x4.extract_lane 2
    local.get $result
    i32x4.extract_lane 3
  )

  ;; Test function for boundary value comparisons
  ;; Tests INT32_MIN, INT32_MAX, and zero boundary conditions
  (func (export "test_i32x4_lt_s_boundary") (param $a0 i32) (param $a1 i32) (param $a2 i32) (param $a3 i32)
                                             (param $b0 i32) (param $b1 i32) (param $b2 i32) (param $b3 i32)
                                             (result i32 i32 i32 i32)
    (local $vec_a v128)
    (local $vec_b v128)
    (local $result v128)

    ;; Build vector A
    local.get $a0
    i32x4.splat
    local.get $a1
    i32x4.replace_lane 1
    local.get $a2
    i32x4.replace_lane 2
    local.get $a3
    i32x4.replace_lane 3
    local.set $vec_a

    ;; Build vector B
    local.get $b0
    i32x4.splat
    local.get $b1
    i32x4.replace_lane 1
    local.get $b2
    i32x4.replace_lane 2
    local.get $b3
    i32x4.replace_lane 3
    local.set $vec_b

    ;; Perform signed less-than comparison
    local.get $vec_a
    local.get $vec_b
    i32x4.lt_s
    local.set $result

    ;; Return result lanes
    local.get $result
    i32x4.extract_lane 0
    local.get $result
    i32x4.extract_lane 1
    local.get $result
    i32x4.extract_lane 2
    local.get $result
    i32x4.extract_lane 3
  )

  ;; Test function for zero operations and identity comparisons
  ;; Tests reflexive property (x < x = false) and zero vector behavior
  (func (export "test_i32x4_lt_s_zero") (param $a0 i32) (param $a1 i32) (param $a2 i32) (param $a3 i32)
                                         (param $b0 i32) (param $b1 i32) (param $b2 i32) (param $b3 i32)
                                         (result i32 i32 i32 i32)
    (local $vec_a v128)
    (local $vec_b v128)
    (local $result v128)

    ;; Build vector A
    local.get $a0
    i32x4.splat
    local.get $a1
    i32x4.replace_lane 1
    local.get $a2
    i32x4.replace_lane 2
    local.get $a3
    i32x4.replace_lane 3
    local.set $vec_a

    ;; Build vector B
    local.get $b0
    i32x4.splat
    local.get $b1
    i32x4.replace_lane 1
    local.get $b2
    i32x4.replace_lane 2
    local.get $b3
    i32x4.replace_lane 3
    local.set $vec_b

    ;; Perform comparison
    local.get $vec_a
    local.get $vec_b
    i32x4.lt_s
    local.set $result

    ;; Return result lanes
    local.get $result
    i32x4.extract_lane 0
    local.get $result
    i32x4.extract_lane 1
    local.get $result
    i32x4.extract_lane 2
    local.get $result
    i32x4.extract_lane 3
  )

  ;; Test function for signed vs unsigned interpretation validation
  ;; Tests values that would differ between signed and unsigned comparison
  (func (export "test_i32x4_lt_s_signed") (param $a0 i32) (param $a1 i32) (param $a2 i32) (param $a3 i32)
                                           (param $b0 i32) (param $b1 i32) (param $b2 i32) (param $b3 i32)
                                           (result i32 i32 i32 i32)
    (local $vec_a v128)
    (local $vec_b v128)
    (local $result v128)

    ;; Build vector A with signed interpretation critical values
    local.get $a0  ;; 0x80000000 (INT32_MIN)
    i32x4.splat
    local.get $a1  ;; 0xFFFFFFFF (-1)
    i32x4.replace_lane 1
    local.get $a2  ;; 0x7FFFFFFF (INT32_MAX)
    i32x4.replace_lane 2
    local.get $a3  ;; 0x80000001 (INT32_MIN + 1)
    i32x4.replace_lane 3
    local.set $vec_a

    ;; Build vector B
    local.get $b0  ;; 0x7FFFFFFF (INT32_MAX)
    i32x4.splat
    local.get $b1  ;; 0x80000000 (INT32_MIN)
    i32x4.replace_lane 1
    local.get $b2  ;; 0xFFFFFFFF (-1)
    i32x4.replace_lane 2
    local.get $b3  ;; 0x7FFFFFFF (INT32_MAX)
    i32x4.replace_lane 3
    local.set $vec_b

    ;; Perform signed comparison (should be different from unsigned)
    local.get $vec_a
    local.get $vec_b
    i32x4.lt_s
    local.set $result

    ;; Return result lanes
    local.get $result
    i32x4.extract_lane 0
    local.get $result
    i32x4.extract_lane 1
    local.get $result
    i32x4.extract_lane 2
    local.get $result
    i32x4.extract_lane 3
  )

  ;; Additional test function for comprehensive edge cases
  ;; Tests mixed scenarios and mathematical properties
  (func (export "test_i32x4_lt_s_comprehensive") (param $a0 i32) (param $a1 i32) (param $a2 i32) (param $a3 i32)
                                                  (param $b0 i32) (param $b1 i32) (param $b2 i32) (param $b3 i32)
                                                  (result i32 i32 i32 i32)
    (local $vec_a v128)
    (local $vec_b v128)
    (local $result v128)

    ;; Build vectors for comprehensive testing
    local.get $a0
    i32x4.splat
    local.get $a1
    i32x4.replace_lane 1
    local.get $a2
    i32x4.replace_lane 2
    local.get $a3
    i32x4.replace_lane 3
    local.set $vec_a

    local.get $b0
    i32x4.splat
    local.get $b1
    i32x4.replace_lane 1
    local.get $b2
    i32x4.replace_lane 2
    local.get $b3
    i32x4.replace_lane 3
    local.set $vec_b

    ;; Perform comparison and return results
    local.get $vec_a
    local.get $vec_b
    i32x4.lt_s
    local.set $result

    local.get $result
    i32x4.extract_lane 0
    local.get $result
    i32x4.extract_lane 1
    local.get $result
    i32x4.extract_lane 2
    local.get $result
    i32x4.extract_lane 3
  )
)