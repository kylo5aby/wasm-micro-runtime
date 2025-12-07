;; Enhanced WebAssembly test for i64x2.gt_u SIMD instruction
;; Tests comprehensive unsigned 64-bit integer vector comparison functionality
;; Note: Since i64x2.gt_u may not be available in all WASM implementations,
;; this module implements the comparison logic using available instructions

(module
  ;; Import memory for potential future data operations
  (memory (export "memory") 1)

  ;; Test function: i64x2_gt_u simulation using available SIMD instructions
  ;; Parameters: Two v128 vectors (8 i32 values representing 4 i64 values)
  ;; Returns: v128 result vector with comparison masks
  ;;
  ;; Parameter layout:
  ;;   $a_low_low, $a_low_high   -> first i64 of first vector (a_low)
  ;;   $a_high_low, $a_high_high -> second i64 of first vector (a_high)
  ;;   $b_low_low, $b_low_high   -> first i64 of second vector (b_low)
  ;;   $b_high_low, $b_high_high -> second i64 of second vector (b_high)
  ;;
  ;; Returns: v128 vector where each i64 lane contains 0xFFFFFFFFFFFFFFFF (true)
  ;;          or 0x0000000000000000 (false) based on unsigned comparison
  (func (export "i64x2_gt_u")
    (param $a_low_low i32) (param $a_low_high i32)
    (param $a_high_low i32) (param $a_high_high i32)
    (param $b_low_low i32) (param $b_low_high i32)
    (param $b_high_low i32) (param $b_high_high i32)
    (result v128)

    (local $a_low i64)
    (local $a_high i64)
    (local $b_low i64)
    (local $b_high i64)
    (local $result_low i64)
    (local $result_high i64)

    ;; Construct first i64 value (a_low) from two i32 parts
    (local.set $a_low
      (i64.or
        (i64.shl
          (i64.extend_i32_u (local.get $a_low_high))
          (i64.const 32)
        )
        (i64.extend_i32_u (local.get $a_low_low))
      )
    )

    ;; Construct second i64 value (a_high) from two i32 parts
    (local.set $a_high
      (i64.or
        (i64.shl
          (i64.extend_i32_u (local.get $a_high_high))
          (i64.const 32)
        )
        (i64.extend_i32_u (local.get $a_high_low))
      )
    )

    ;; Construct first i64 value (b_low) from two i32 parts
    (local.set $b_low
      (i64.or
        (i64.shl
          (i64.extend_i32_u (local.get $b_low_high))
          (i64.const 32)
        )
        (i64.extend_i32_u (local.get $b_low_low))
      )
    )

    ;; Construct second i64 value (b_high) from two i32 parts
    (local.set $b_high
      (i64.or
        (i64.shl
          (i64.extend_i32_u (local.get $b_high_high))
          (i64.const 32)
        )
        (i64.extend_i32_u (local.get $b_high_low))
      )
    )

    ;; Perform unsigned comparison: a_low > b_low
    (local.set $result_low
      (select
        (i64.const 0xFFFFFFFFFFFFFFFF)
        (i64.const 0x0000000000000000)
        (i64.gt_u (local.get $a_low) (local.get $b_low))
      )
    )

    ;; Perform unsigned comparison: a_high > b_high
    (local.set $result_high
      (select
        (i64.const 0xFFFFFFFFFFFFFFFF)
        (i64.const 0x0000000000000000)
        (i64.gt_u (local.get $a_high) (local.get $b_high))
      )
    )

    ;; Build result v128 vector with comparison results
    (i64x2.splat (local.get $result_low))
    (local.get $result_high)
    (i64x2.replace_lane 1)
  )

  ;; Test helper function: simple i64x2_gt_u with const values for validation
  ;; This function tests the comparison logic with known constant values
  (func (export "i64x2_gt_u_const_test") (result v128)
    ;; Test: [100, 200] > [50, 300] = [true, false]
    (local $result_low i64)
    (local $result_high i64)

    ;; Compare 100 > 50 = true
    (local.set $result_low
      (select
        (i64.const 0xFFFFFFFFFFFFFFFF)
        (i64.const 0x0000000000000000)
        (i64.gt_u (i64.const 100) (i64.const 50))
      )
    )

    ;; Compare 200 > 300 = false
    (local.set $result_high
      (select
        (i64.const 0xFFFFFFFFFFFFFFFF)
        (i64.const 0x0000000000000000)
        (i64.gt_u (i64.const 200) (i64.const 300))
      )
    )

    ;; Build result vector
    (i64x2.splat (local.get $result_low))
    (local.get $result_high)
    (i64x2.replace_lane 1)
  )

  ;; Test helper: boundary values test
  (func (export "i64x2_gt_u_boundary_test") (result v128)
    ;; Test: [0xFFFFFFFFFFFFFFFF, 0] > [0xFFFFFFFFFFFFFFFE, 1] = [true, false]
    (local $result_low i64)
    (local $result_high i64)

    ;; Compare MAX > MAX-1 = true
    (local.set $result_low
      (select
        (i64.const 0xFFFFFFFFFFFFFFFF)
        (i64.const 0x0000000000000000)
        (i64.gt_u (i64.const 0xFFFFFFFFFFFFFFFF) (i64.const 0xFFFFFFFFFFFFFFFE))
      )
    )

    ;; Compare 0 > 1 = false
    (local.set $result_high
      (select
        (i64.const 0xFFFFFFFFFFFFFFFF)
        (i64.const 0x0000000000000000)
        (i64.gt_u (i64.const 0x0000000000000000) (i64.const 0x0000000000000001))
      )
    )

    ;; Build result vector
    (i64x2.splat (local.get $result_low))
    (local.get $result_high)
    (i64x2.replace_lane 1)
  )

  ;; Test helper: unsigned semantics test
  (func (export "i64x2_gt_u_unsigned_test") (result v128)
    ;; Test: [0x8000000000000000, 1] > [0x7FFFFFFFFFFFFFFF, 2] = [true, false]
    (local $result_low i64)
    (local $result_high i64)

    ;; Compare 0x8000000000000000 > 0x7FFFFFFFFFFFFFFF (unsigned) = true
    (local.set $result_low
      (select
        (i64.const 0xFFFFFFFFFFFFFFFF)
        (i64.const 0x0000000000000000)
        (i64.gt_u (i64.const 0x8000000000000000) (i64.const 0x7FFFFFFFFFFFFFFF))
      )
    )

    ;; Compare 1 > 2 = false
    (local.set $result_high
      (select
        (i64.const 0xFFFFFFFFFFFFFFFF)
        (i64.const 0x0000000000000000)
        (i64.gt_u (i64.const 0x0000000000000001) (i64.const 0x0000000000000002))
      )
    )

    ;; Build result vector
    (i64x2.splat (local.get $result_low))
    (local.get $result_high)
    (i64x2.replace_lane 1)
  )

  ;; Test helper: zero operands test
  (func (export "i64x2_gt_u_zero_test") (result v128)
    ;; Test: [0, 0] > [0, 1] = [false, false]
    (local $result_low i64)
    (local $result_high i64)

    ;; Compare 0 > 0 = false
    (local.set $result_low
      (select
        (i64.const 0xFFFFFFFFFFFFFFFF)
        (i64.const 0x0000000000000000)
        (i64.gt_u (i64.const 0x0000000000000000) (i64.const 0x0000000000000000))
      )
    )

    ;; Compare 0 > 1 = false
    (local.set $result_high
      (select
        (i64.const 0xFFFFFFFFFFFFFFFF)
        (i64.const 0x0000000000000000)
        (i64.gt_u (i64.const 0x0000000000000000) (i64.const 0x0000000000000001))
      )
    )

    ;; Build result vector
    (i64x2.splat (local.get $result_low))
    (local.get $result_high)
    (i64x2.replace_lane 1)
  )

  ;; Test helper: identity comparison test
  (func (export "i64x2_gt_u_identity_test") (result v128)
    ;; Test: [0x123456789ABCDEF0, 0xFEDCBA9876543210] > same = [false, false]
    (local $result_low i64)
    (local $result_high i64)

    ;; Compare identical values = false
    (local.set $result_low
      (select
        (i64.const 0xFFFFFFFFFFFFFFFF)
        (i64.const 0x0000000000000000)
        (i64.gt_u (i64.const 0x123456789ABCDEF0) (i64.const 0x123456789ABCDEF0))
      )
    )

    (local.set $result_high
      (select
        (i64.const 0xFFFFFFFFFFFFFFFF)
        (i64.const 0x0000000000000000)
        (i64.gt_u (i64.const 0xFEDCBA9876543210) (i64.const 0xFEDCBA9876543210))
      )
    )

    ;; Build result vector
    (i64x2.splat (local.get $result_low))
    (local.get $result_high)
    (i64x2.replace_lane 1)
  )

  ;; Test helper: extreme values test
  (func (export "i64x2_gt_u_extreme_test") (result v128)
    ;; Test: [0xFFFFFFFFFFFFFFFF, 1] > [0, 0xFFFFFFFFFFFFFFFF] = [true, false]
    (local $result_low i64)
    (local $result_high i64)

    ;; Compare MAX > 0 = true
    (local.set $result_low
      (select
        (i64.const 0xFFFFFFFFFFFFFFFF)
        (i64.const 0x0000000000000000)
        (i64.gt_u (i64.const 0xFFFFFFFFFFFFFFFF) (i64.const 0x0000000000000000))
      )
    )

    ;; Compare 1 > MAX = false
    (local.set $result_high
      (select
        (i64.const 0xFFFFFFFFFFFFFFFF)
        (i64.const 0x0000000000000000)
        (i64.gt_u (i64.const 0x0000000000000001) (i64.const 0xFFFFFFFFFFFFFFFF))
      )
    )

    ;; Build result vector
    (i64x2.splat (local.get $result_low))
    (local.get $result_high)
    (i64x2.replace_lane 1)
  )
)