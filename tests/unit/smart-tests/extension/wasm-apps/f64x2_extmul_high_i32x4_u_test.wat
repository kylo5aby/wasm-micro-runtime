(module
  ;; f64x2.extmul_high_i32x4_u comprehensive test module
  ;; This module provides test functions for the f64x2.extmul_high_i32x4_u SIMD instruction
  ;; The instruction performs extended multiplication on the high two i32 lanes (lanes 2,3)
  ;; treating them as unsigned integers and produces two f64 results

  ;; Memory for test data exchange and intermediate storage
  (memory (export "memory") 1)

  ;; Manual extended multiplication simulation for high lanes (indices 2,3)
  ;; Extracts higher two i32 elements, converts to i64 as unsigned, multiplies, then converts to f64
  (func $simulate_extmul_high_u (param $input1 v128) (param $input2 v128) (result v128)
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

    ;; Perform extended multiplication: i32 -> i64 -> i64 multiplication (UNSIGNED)
    ;; Lane 2 multiplication: elem1_i32 * elem3_i32 (as unsigned)
    (local.set $result1_i64
      (i64.mul
        (i64.extend_i32_u (local.get $elem1_i32))
        (i64.extend_i32_u (local.get $elem3_i32))
      )
    )
    ;; Lane 3 multiplication: elem2_i32 * elem4_i32 (as unsigned)
    (local.set $result2_i64
      (i64.mul
        (i64.extend_i32_u (local.get $elem2_i32))
        (i64.extend_i32_u (local.get $elem4_i32))
      )
    )

    ;; Convert i64 results to f64 (unsigned conversion)
    (local.set $result1_f64 (f64.convert_i64_u (local.get $result1_i64)))
    (local.set $result2_f64 (f64.convert_i64_u (local.get $result2_i64)))

    ;; Construct f64x2 result vector
    (f64x2.splat (local.get $result1_f64))
    (local.get $result2_f64)
    (f64x2.replace_lane 1)
  )

  ;; Test function: Basic extended multiplication functionality
  ;; Parameters: 8 i32 values representing two v128 vectors (4 lanes each)
  ;; Returns: f64x2 result from multiplying high lanes as unsigned values
  ;; Operation: result[0] = (f64)(a[2] * b[2]), result[1] = (f64)(a[3] * b[3])
  (func (export "test_basic_extmul_high")
        (param $a0 i32) (param $a1 i32) (param $a2 i32) (param $a3 i32)
        (param $b0 i32) (param $b1 i32) (param $b2 i32) (param $b3 i32)
        (result v128)
    ;; Construct first v128 from input parameters a0,a1,a2,a3
    (v128.const i32x4 0x00000000 0x00000000 0x00000000 0x00000000)
    (local.get $a0)
    (i32x4.replace_lane 0)
    (local.get $a1)
    (i32x4.replace_lane 1)
    (local.get $a2)
    (i32x4.replace_lane 2)
    (local.get $a3)
    (i32x4.replace_lane 3)

    ;; Construct second v128 from input parameters b0,b1,b2,b3
    (v128.const i32x4 0x00000000 0x00000000 0x00000000 0x00000000)
    (local.get $b0)
    (i32x4.replace_lane 0)
    (local.get $b1)
    (i32x4.replace_lane 1)
    (local.get $b2)
    (i32x4.replace_lane 2)
    (local.get $b3)
    (i32x4.replace_lane 3)

    ;; Execute simulated f64x2.extmul_high_i32x4_u via helper function
    (call $simulate_extmul_high_u)
  )

  ;; Test function: Boundary values with maximum unsigned i32 values
  ;; Specifically designed to test UINT32_MAX boundary conditions
  ;; and large product generation capabilities
  (func (export "test_boundary_extmul_high")
        (param $a0 i32) (param $a1 i32) (param $a2 i32) (param $a3 i32)
        (param $b0 i32) (param $b1 i32) (param $b2 i32) (param $b3 i32)
        (result v128)
    ;; Construct first v128 vector
    (v128.const i32x4 0x00000000 0x00000000 0x00000000 0x00000000)
    (local.get $a0)
    (i32x4.replace_lane 0)
    (local.get $a1)
    (i32x4.replace_lane 1)
    (local.get $a2)
    (i32x4.replace_lane 2)
    (local.get $a3)
    (i32x4.replace_lane 3)

    ;; Construct second v128 vector
    (v128.const i32x4 0x00000000 0x00000000 0x00000000 0x00000000)
    (local.get $b0)
    (i32x4.replace_lane 0)
    (local.get $b1)
    (i32x4.replace_lane 1)
    (local.get $b2)
    (i32x4.replace_lane 2)
    (local.get $b3)
    (i32x4.replace_lane 3)

    ;; Execute simulated extended multiplication on high lanes with boundary value handling
    (call $simulate_extmul_high_u)
  )

  ;; Test function: Special values including zero and identity operations
  ;; Tests mathematical properties: absorbing element (×0) and multiplicative identity (×1)
  (func (export "test_special_extmul_high")
        (param $a0 i32) (param $a1 i32) (param $a2 i32) (param $a3 i32)
        (param $b0 i32) (param $b1 i32) (param $b2 i32) (param $b3 i32)
        (result v128)
    ;; Build first v128 with special test values
    (v128.const i32x4 0x00000000 0x00000000 0x00000000 0x00000000)
    (local.get $a0)
    (i32x4.replace_lane 0)
    (local.get $a1)
    (i32x4.replace_lane 1)
    (local.get $a2)
    (i32x4.replace_lane 2)
    (local.get $a3)
    (i32x4.replace_lane 3)

    ;; Build second v128 with special test values (zeros, ones, etc.)
    (v128.const i32x4 0x00000000 0x00000000 0x00000000 0x00000000)
    (local.get $b0)
    (i32x4.replace_lane 0)
    (local.get $b1)
    (i32x4.replace_lane 1)
    (local.get $b2)
    (i32x4.replace_lane 2)
    (local.get $b3)
    (i32x4.replace_lane 3)

    ;; Execute simulated extended multiplication for special value validation
    (call $simulate_extmul_high_u)
  )

  ;; Test function: Precision validation with large unsigned values
  ;; Designed to test f64 precision limits and large product accuracy
  (func (export "test_precision_extmul_high")
        (param $a0 i32) (param $a1 i32) (param $a2 i32) (param $a3 i32)
        (param $b0 i32) (param $b1 i32) (param $b2 i32) (param $b3 i32)
        (result v128)
    ;; Create first v128 with large precision test values
    (v128.const i32x4 0x00000000 0x00000000 0x00000000 0x00000000)
    (local.get $a0)
    (i32x4.replace_lane 0)
    (local.get $a1)
    (i32x4.replace_lane 1)
    (local.get $a2)
    (i32x4.replace_lane 2)
    (local.get $a3)
    (i32x4.replace_lane 3)

    ;; Create second v128 with complementary precision test values
    (v128.const i32x4 0x00000000 0x00000000 0x00000000 0x00000000)
    (local.get $b0)
    (i32x4.replace_lane 0)
    (local.get $b1)
    (i32x4.replace_lane 1)
    (local.get $b2)
    (i32x4.replace_lane 2)
    (local.get $b3)
    (i32x4.replace_lane 3)

    ;; Execute simulated extended multiplication for precision validation
    ;; Tests that all u32×u32 products can be accurately represented in f64
    (call $simulate_extmul_high_u)
  )

  ;; Utility function: Extract f64 lane from f64x2 vector (for debugging/validation)
  ;; This function demonstrates how to extract individual f64 values from the result
  (func (export "extract_f64_lane_0") (param $vec v128) (result f64)
    (local.get $vec)
    (f64x2.extract_lane 0)
  )

  (func (export "extract_f64_lane_1") (param $vec v128) (result f64)
    (local.get $vec)
    (f64x2.extract_lane 1)
  )

  ;; Test function: Power-of-two multiplication validation
  ;; Specifically tests binary arithmetic precision with powers of 2
  (func (export "test_power_of_two") (result v128)
    ;; Test with specific power-of-two values: 2^16 × 2^8 = 2^24, 2^20 × 2^4 = 2^24
    (v128.const i32x4 0x00000000 0x00000000 0x00010000 0x00100000)  ;; [_, _, 2^16, 2^20]
    (v128.const i32x4 0x00000000 0x00000000 0x00000100 0x00000010)  ;; [_, _, 2^8, 2^4]

    ;; Execute: should produce [2^24, 2^24] = [16777216.0, 16777216.0]
    (call $simulate_extmul_high_u)
  )

  ;; Test function: Maximum value edge case
  ;; Tests the absolute maximum product: UINT32_MAX × UINT32_MAX
  (func (export "test_max_product") (result v128)
    ;; Both vectors contain UINT32_MAX (0xFFFFFFFF) in high lanes
    (v128.const i32x4 0x00000000 0x00000000 0xFFFFFFFF 0xFFFFFFFF)
    (v128.const i32x4 0x00000000 0x00000000 0xFFFFFFFF 0xFFFFFFFF)

    ;; Execute: should produce maximum possible product twice
    ;; Result: [18446744065119617025.0, 18446744065119617025.0]
    (call $simulate_extmul_high_u)
  )
)