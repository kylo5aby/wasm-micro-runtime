(module
  ;;
  ;; Test functions for f32x4.replace_lane opcode validation
  ;; Each function creates an f32x4 vector from 4 f32 inputs and replaces a specific lane
  ;;

  ;;
  ;; Test function: replace lane 0 in f32x4 vector
  ;;
  (func (export "test_replace_lane_0")
    (param $v0 f32) (param $v1 f32) (param $v2 f32) (param $v3 f32)
    (param $replacement f32)
    (result v128)
    ;; Build f32x4 vector from input parameters
    (f32x4.splat (local.get $v0))
    (local.get $v1)
    (f32x4.replace_lane 1)
    (local.get $v2)
    (f32x4.replace_lane 2)
    (local.get $v3)
    (f32x4.replace_lane 3)
    ;; Replace lane 0 with replacement value
    (local.get $replacement)
    (f32x4.replace_lane 0)
  )

  ;;
  ;; Test function: replace lane 1 in f32x4 vector
  ;;
  (func (export "test_replace_lane_1")
    (param $v0 f32) (param $v1 f32) (param $v2 f32) (param $v3 f32)
    (param $replacement f32)
    (result v128)
    ;; Build f32x4 vector from input parameters
    (f32x4.splat (local.get $v0))
    (local.get $v1)
    (f32x4.replace_lane 1)
    (local.get $v2)
    (f32x4.replace_lane 2)
    (local.get $v3)
    (f32x4.replace_lane 3)
    ;; Replace lane 1 with replacement value
    (local.get $replacement)
    (f32x4.replace_lane 1)
  )

  ;;
  ;; Test function: replace lane 2 in f32x4 vector
  ;;
  (func (export "test_replace_lane_2")
    (param $v0 f32) (param $v1 f32) (param $v2 f32) (param $v3 f32)
    (param $replacement f32)
    (result v128)
    ;; Build f32x4 vector from input parameters
    (f32x4.splat (local.get $v0))
    (local.get $v1)
    (f32x4.replace_lane 1)
    (local.get $v2)
    (f32x4.replace_lane 2)
    (local.get $v3)
    (f32x4.replace_lane 3)
    ;; Replace lane 2 with replacement value
    (local.get $replacement)
    (f32x4.replace_lane 2)
  )

  ;;
  ;; Test function: replace lane 3 in f32x4 vector
  ;;
  (func (export "test_replace_lane_3")
    (param $v0 f32) (param $v1 f32) (param $v2 f32) (param $v3 f32)
    (param $replacement f32)
    (result v128)
    ;; Build f32x4 vector from input parameters
    (f32x4.splat (local.get $v0))
    (local.get $v1)
    (f32x4.replace_lane 1)
    (local.get $v2)
    (f32x4.replace_lane 2)
    (local.get $v3)
    (f32x4.replace_lane 3)
    ;; Replace lane 3 with replacement value
    (local.get $replacement)
    (f32x4.replace_lane 3)
  )

  ;;
  ;; Test function: boundary values test - replace lane 0 with FLT_MAX equivalent
  ;;
  (func (export "test_boundary_flt_max")
    (param $v0 f32) (param $v1 f32) (param $v2 f32) (param $v3 f32)
    (result v128)
    ;; Build f32x4 vector from input parameters
    (f32x4.splat (local.get $v0))
    (local.get $v1)
    (f32x4.replace_lane 1)
    (local.get $v2)
    (f32x4.replace_lane 2)
    (local.get $v3)
    (f32x4.replace_lane 3)
    ;; Replace lane 0 with maximum f32 value
    (f32.const 3.4028235e+38)  ;; FLT_MAX approximation
    (f32x4.replace_lane 0)
  )

  ;;
  ;; Test function: boundary values test - replace lane 1 with FLT_MIN equivalent
  ;;
  (func (export "test_boundary_flt_min")
    (param $v0 f32) (param $v1 f32) (param $v2 f32) (param $v3 f32)
    (result v128)
    ;; Build f32x4 vector from input parameters
    (f32x4.splat (local.get $v0))
    (local.get $v1)
    (f32x4.replace_lane 1)
    (local.get $v2)
    (f32x4.replace_lane 2)
    (local.get $v3)
    (f32x4.replace_lane 3)
    ;; Replace lane 1 with minimum positive normalized f32 value
    (f32.const 1.1754944e-38)  ;; FLT_MIN approximation
    (f32x4.replace_lane 1)
  )

  ;;
  ;; Test function: special values test - replace lanes with NaN
  ;;
  (func (export "test_special_nan")
    (param $v0 f32) (param $v1 f32) (param $v2 f32) (param $v3 f32)
    (result v128)
    ;; Build f32x4 vector from input parameters
    (f32x4.splat (local.get $v0))
    (local.get $v1)
    (f32x4.replace_lane 1)
    (local.get $v2)
    (f32x4.replace_lane 2)
    (local.get $v3)
    (f32x4.replace_lane 3)
    ;; Replace lane 0 with NaN
    (f32.const nan)
    (f32x4.replace_lane 0)
  )

  ;;
  ;; Test function: special values test - replace lanes with +infinity
  ;;
  (func (export "test_special_pos_inf")
    (param $v0 f32) (param $v1 f32) (param $v2 f32) (param $v3 f32)
    (result v128)
    ;; Build f32x4 vector from input parameters
    (f32x4.splat (local.get $v0))
    (local.get $v1)
    (f32x4.replace_lane 1)
    (local.get $v2)
    (f32x4.replace_lane 2)
    (local.get $v3)
    (f32x4.replace_lane 3)
    ;; Replace lane 1 with positive infinity
    (f32.const inf)
    (f32x4.replace_lane 1)
  )

  ;;
  ;; Test function: special values test - replace lanes with -infinity
  ;;
  (func (export "test_special_neg_inf")
    (param $v0 f32) (param $v1 f32) (param $v2 f32) (param $v3 f32)
    (result v128)
    ;; Build f32x4 vector from input parameters
    (f32x4.splat (local.get $v0))
    (local.get $v1)
    (f32x4.replace_lane 1)
    (local.get $v2)
    (f32x4.replace_lane 2)
    (local.get $v3)
    (f32x4.replace_lane 3)
    ;; Replace lane 2 with negative infinity
    (f32.const -inf)
    (f32x4.replace_lane 2)
  )

  ;;
  ;; Test function: special values test - replace lanes with -0.0
  ;;
  (func (export "test_special_neg_zero")
    (param $v0 f32) (param $v1 f32) (param $v2 f32) (param $v3 f32)
    (result v128)
    ;; Build f32x4 vector from input parameters
    (f32x4.splat (local.get $v0))
    (local.get $v1)
    (f32x4.replace_lane 1)
    (local.get $v2)
    (f32x4.replace_lane 2)
    (local.get $v3)
    (f32x4.replace_lane 3)
    ;; Replace lane 3 with negative zero
    (f32.const -0.0)
    (f32x4.replace_lane 3)
  )

  ;;
  ;; Test function: identity replacement test - replace lane with same value
  ;;
  (func (export "test_identity_replacement")
    (param $v0 f32) (param $v1 f32) (param $v2 f32) (param $v3 f32)
    (result v128)
    ;; Build f32x4 vector from input parameters
    (f32x4.splat (local.get $v0))
    (local.get $v1)
    (f32x4.replace_lane 1)
    (local.get $v2)
    (f32x4.replace_lane 2)
    (local.get $v3)
    (f32x4.replace_lane 3)
    ;; Replace lane 1 with same value (identity operation)
    (local.get $v1)
    (f32x4.replace_lane 1)
  )

  ;;
  ;; Test function: comprehensive lane replacement - all lanes with different values
  ;;
  (func (export "test_comprehensive_replacement")
    (result v128)
    ;; Start with a base vector [1.0, 2.0, 3.0, 4.0]
    (f32x4.splat (f32.const 1.0))
    (f32.const 2.0)
    (f32x4.replace_lane 1)
    (f32.const 3.0)
    (f32x4.replace_lane 2)
    (f32.const 4.0)
    (f32x4.replace_lane 3)

    ;; Replace all lanes with new values: [42.0, -15.5, 3.14159, 0.0]
    (f32.const 42.0)
    (f32x4.replace_lane 0)
    (f32.const -15.5)
    (f32x4.replace_lane 1)
    (f32.const 3.14159)
    (f32x4.replace_lane 2)
    (f32.const 0.0)
    (f32x4.replace_lane 3)
  )
)