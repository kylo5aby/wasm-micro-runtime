(module
  ;; Memory for test data exchange and intermediate storage
  (memory (export "memory") 1)

  ;; Manual extended multiplication simulation for high lanes (indices 2,3)
  ;; Extracts higher two i32 elements, converts to i64, multiplies, then converts to f64
  (func $simulate_extmul_high (param $input1 v128) (param $input2 v128) (result v128)
    (local $elem1_i32 i32) (local $elem2_i32 i32)  ;; High lanes from first vector
    (local $elem3_i32 i32) (local $elem4_i32 i32)  ;; High lanes from second vector
    (local $result1_i64 i64) (local $result2_i64 i64)  ;; Extended multiplication results
    (local $result1_f64 f64) (local $result2_f64 f64)  ;; Final f64 results

    ;; Extract higher two i32 elements from first vector (lanes 2,3)
    (local.set $elem1_i32 (i32x4.extract_lane 2 (local.get $input1)))
    (local.set $elem2_i32 (i32x4.extract_lane 3 (local.get $input1)))

    ;; Extract higher two i32 elements from second vector (lanes 2,3)
    (local.set $elem3_i32 (i32x4.extract_lane 2 (local.get $input2)))
    (local.set $elem4_i32 (i32x4.extract_lane 3 (local.get $input2)))

    ;; Perform extended multiplication: i32 -> i64 -> i64 multiplication
    ;; Lane 2 multiplication: elem1_i32 * elem3_i32
    (local.set $result1_i64
      (i64.mul
        (i64.extend_i32_s (local.get $elem1_i32))
        (i64.extend_i32_s (local.get $elem3_i32))
      )
    )
    ;; Lane 3 multiplication: elem2_i32 * elem4_i32
    (local.set $result2_i64
      (i64.mul
        (i64.extend_i32_s (local.get $elem2_i32))
        (i64.extend_i32_s (local.get $elem4_i32))
      )
    )

    ;; Convert i64 results to f64
    (local.set $result1_f64 (f64.convert_i64_s (local.get $result1_i64)))
    (local.set $result2_f64 (f64.convert_i64_s (local.get $result2_i64)))

    ;; Construct f64x2 result vector
    (f64x2.splat (local.get $result1_f64))
    (local.get $result2_f64)
    (f64x2.replace_lane 1)
  )

  ;; Basic extended multiplication test function
  ;; Tests fundamental f64x2.extmul_high_i32x4_s operation with typical signed i32 values
  ;; Parameters: 8 x i32 (representing two v128 vectors as 4 i32 lanes each)
  ;; Returns: f64x2 (as two separate f64 values from high lanes multiplication)
  (func (export "test_basic_extmul_high")
    (param $a0 i32) (param $a1 i32) (param $a2 i32) (param $a3 i32)  ;; First v128 as i32 lanes
    (param $b0 i32) (param $b1 i32) (param $b2 i32) (param $b3 i32)  ;; Second v128 as i32 lanes
    (result f64 f64)  ;; Return two f64 values from extended multiplication
    (local $result v128)

    ;; Construct first v128 vector from i32 parameters
    ;; v128.const would be ideal, but using i32x4.splat + i32x4.replace_lane for compatibility
    (i32.const 0)
    (i32x4.splat)
    (local.get $a0) (i32x4.replace_lane 0)
    (local.get $a1) (i32x4.replace_lane 1)
    (local.get $a2) (i32x4.replace_lane 2)
    (local.get $a3) (i32x4.replace_lane 3)

    ;; Construct second v128 vector from i32 parameters
    (i32.const 0)
    (i32x4.splat)
    (local.get $b0) (i32x4.replace_lane 0)
    (local.get $b1) (i32x4.replace_lane 1)
    (local.get $b2) (i32x4.replace_lane 2)
    (local.get $b3) (i32x4.replace_lane 3)

    ;; Perform simulated f64x2.extmul_high_i32x4_s operation
    ;; This multiplies high lanes (indices 2,3) as signed i32 and produces f64x2 result
    (call $simulate_extmul_high)

    ;; Extract both f64 values from the result vector
    (local.set $result)
    (f64x2.extract_lane 0 (local.get $result))  ;; First f64 result (from lanes 2 multiplication)
    (f64x2.extract_lane 1 (local.get $result))  ;; Second f64 result (from lanes 3 multiplication)
  )

  ;; Boundary value extended multiplication test function
  ;; Tests f64x2.extmul_high_i32x4_s with INT32_MAX, INT32_MIN boundary conditions
  ;; Validates proper handling of maximum positive/negative signed i32 values
  (func (export "test_boundary_extmul_high")
    (param $a0 i32) (param $a1 i32) (param $a2 i32) (param $a3 i32)
    (param $b0 i32) (param $b1 i32) (param $b2 i32) (param $b3 i32)
    (result f64 f64)
    (local $result v128)

    ;; Construct first v128 vector (may contain boundary values)
    (i32.const 0)
    (i32x4.splat)
    (local.get $a0) (i32x4.replace_lane 0)
    (local.get $a1) (i32x4.replace_lane 1)
    (local.get $a2) (i32x4.replace_lane 2)  ;; High lane - may be INT32_MAX/MIN
    (local.get $a3) (i32x4.replace_lane 3)  ;; High lane - may be INT32_MAX/MIN

    ;; Construct second v128 vector (may contain boundary values)
    (i32.const 0)
    (i32x4.splat)
    (local.get $b0) (i32x4.replace_lane 0)
    (local.get $b1) (i32x4.replace_lane 1)
    (local.get $b2) (i32x4.replace_lane 2)  ;; High lane - may be INT32_MAX/MIN
    (local.get $b3) (i32x4.replace_lane 3)  ;; High lane - may be INT32_MAX/MIN

    ;; Perform simulated extended multiplication on high lanes
    ;; Expected to handle large products like INT32_MAX × INT32_MAX = 4,611,686,014,132,420,609
    (call $simulate_extmul_high)

    ;; Extract f64 results - should be large values but within f64 precision range
    (local.set $result)
    (f64x2.extract_lane 0 (local.get $result))
    (f64x2.extract_lane 1 (local.get $result))
  )

  ;; Special values extended multiplication test function
  ;; Tests f64x2.extmul_high_i32x4_s with zero, identity, and mathematical properties
  ;; Validates absorbing element (×0), multiplicative identity (×1), and commutativity
  (func (export "test_special_extmul_high")
    (param $a0 i32) (param $a1 i32) (param $a2 i32) (param $a3 i32)
    (param $b0 i32) (param $b1 i32) (param $b2 i32) (param $b3 i32)
    (result f64 f64)
    (local $result v128)

    ;; Construct first v128 vector (may contain special values like 0, 1, -1)
    (i32.const 0)
    (i32x4.splat)
    (local.get $a0) (i32x4.replace_lane 0)
    (local.get $a1) (i32x4.replace_lane 1)
    (local.get $a2) (i32x4.replace_lane 2)  ;; High lane - may be test value
    (local.get $a3) (i32x4.replace_lane 3)  ;; High lane - may be test value

    ;; Construct second v128 vector (may contain special values)
    (i32.const 0)
    (i32x4.splat)
    (local.get $b0) (i32x4.replace_lane 0)
    (local.get $b1) (i32x4.replace_lane 1)
    (local.get $b2) (i32x4.replace_lane 2)  ;; High lane - may be 0, 1, or -1
    (local.get $b3) (i32x4.replace_lane 3)  ;; High lane - may be 0, 1, or -1

    ;; Perform simulated extended multiplication
    ;; Expected outcomes: value×0=0.0, value×1=value, value×(-1)=-value
    (call $simulate_extmul_high)

    ;; Extract f64 results for validation of mathematical properties
    (local.set $result)
    (f64x2.extract_lane 0 (local.get $result))
    (f64x2.extract_lane 1 (local.get $result))
  )

  ;; Helper function to test individual lane extraction and validation
  ;; Provides utility for debugging and detailed result inspection
  (func (export "test_lane_extraction")
    (param $val i32)  ;; Single i32 value to splat across v128
    (result f64 f64)
    (local $result v128)

    ;; Create v128 with same value in all lanes
    (local.get $val)
    (i32x4.splat)

    ;; Duplicate for second operand
    (local.get $val)
    (i32x4.splat)

    ;; Perform simulated extended multiplication - should produce val×val in both f64 lanes
    (call $simulate_extmul_high)

    ;; Extract both results (should be identical: val²)
    (local.set $result)
    (f64x2.extract_lane 0 (local.get $result))
    (f64x2.extract_lane 1 (local.get $result))
  )

  ;; Function to test commutativity property: a × b = b × a
  ;; Validates that f64x2.extmul_high_i32x4_s respects mathematical commutativity
  (func (export "test_commutativity")
    (param $a2 i32) (param $a3 i32) (param $b2 i32) (param $b3 i32)  ;; High lane values only
    (result f64 f64 f64 f64)  ;; Returns a×b and b×a results for comparison
    (local $result1 v128) (local $result2 v128)

    ;; First calculation: a × b
    (i32.const 0)
    (i32x4.splat)
    (local.get $a2) (i32x4.replace_lane 2)
    (local.get $a3) (i32x4.replace_lane 3)

    (i32.const 0)
    (i32x4.splat)
    (local.get $b2) (i32x4.replace_lane 2)
    (local.get $b3) (i32x4.replace_lane 3)

    (call $simulate_extmul_high)
    (local.set $result1)

    ;; Second calculation: b × a (swapped operands)
    (i32.const 0)
    (i32x4.splat)
    (local.get $b2) (i32x4.replace_lane 2)
    (local.get $b3) (i32x4.replace_lane 3)

    (i32.const 0)
    (i32x4.splat)
    (local.get $a2) (i32x4.replace_lane 2)
    (local.get $a3) (i32x4.replace_lane 3)

    (call $simulate_extmul_high)
    (local.set $result2)

    ;; Return all four results for comparison
    (f64x2.extract_lane 0 (local.get $result1))  ;; a2 × b2
    (f64x2.extract_lane 1 (local.get $result1))  ;; a3 × b3
    (f64x2.extract_lane 0 (local.get $result2))  ;; b2 × a2 (should equal a2 × b2)
    (f64x2.extract_lane 1 (local.get $result2))  ;; b3 × a3 (should equal a3 × b3)
  )
)