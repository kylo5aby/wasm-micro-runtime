(module
  ;; Test function 1: Basic extended multiplication for high lanes (unsigned)
  ;; Takes two v128 parameters and returns v128 result
  ;; Performs i32x4.extmul_high_i16x8_u on the inputs (operates on lanes 4-7)
  (func (export "test_basic_extmul") (param v128 v128) (result v128)
    local.get 0  ;; Load first v128 parameter (u16x8)
    local.get 1  ;; Load second v128 parameter (u16x8)
    i32x4.extmul_high_i16x8_u  ;; Perform unsigned extended multiplication on higher 4 lanes (4-7)
  )

  ;; Test function 2: Boundary values test for high lanes (unsigned)
  ;; Tests u16 MIN/MAX boundary conditions in high lanes
  (func (export "test_boundary_extmul") (param v128 v128) (result v128)
    local.get 0
    local.get 1
    i32x4.extmul_high_i16x8_u
  )

  ;; Test function 3: Zero and identity operations test for high lanes (unsigned)
  ;; Tests zero multiplication property (0 × n = 0) and identity (1 × n = n) in high lanes
  (func (export "test_zero_extmul") (param v128 v128) (result v128)
    local.get 0
    local.get 1
    i32x4.extmul_high_i16x8_u
  )

  ;; Test function 4: SIMD module validation test for high lanes (unsigned)
  ;; Tests proper SIMD instruction handling and execution
  (func (export "test_validation_extmul") (param v128 v128) (result v128)
    local.get 0
    local.get 1
    i32x4.extmul_high_i16x8_u
  )

  ;; Test function 5: Comprehensive unsigned test with various inputs in high lanes
  ;; Tests multiple unsigned scenarios in a single function
  (func (export "test_comprehensive_extmul") (param v128 v128) (result v128)
    local.get 0
    local.get 1
    i32x4.extmul_high_i16x8_u
  )

  ;; Test function 6: Lane independence test for high lanes (unsigned)
  ;; Verifies that each high lane is computed independently
  (func (export "test_lane_independence") (param v128 v128) (result v128)
    local.get 0
    local.get 1
    i32x4.extmul_high_i16x8_u
  )

  ;; Test function 7: Maximum value test for unsigned high lanes
  ;; Tests maximum u16 values and their products
  (func (export "test_maximum_values") (param v128 v128) (result v128)
    local.get 0
    local.get 1
    i32x4.extmul_high_i16x8_u
  )

  ;; Test function 8: Cross-mode consistency test for high lanes (unsigned)
  ;; Ensures identical results between interpreter and AOT modes
  (func (export "test_cross_mode_consistency") (param v128 v128) (result v128)
    local.get 0
    local.get 1
    i32x4.extmul_high_i16x8_u
  )

  ;; Test constant vectors for specific test cases (focusing on high lanes 4-7, unsigned)
  ;; Vector with small positive values for basic testing in high lanes
  (func (export "get_basic_test_vector1") (result v128)
    ;; u16x8: [0, 0, 0, 0, 10, 20, 30, 40] (high lanes 4-7: 10, 20, 30, 40)
    ;; Packed as i32x4: [0x00000000, 0x00000000, 0x0014000A, 0x0028001E]
    v128.const i32x4 0x00000000 0x00000000 0x0014000A 0x0028001E
  )

  (func (export "get_basic_test_vector2") (result v128)
    ;; u16x8: [0, 0, 0, 0, 2, 3, 4, 5] (high lanes 4-7: 2, 3, 4, 5)
    ;; Packed as i32x4: [0x00000000, 0x00000000, 0x00030002, 0x00050004]
    v128.const i32x4 0x00000000 0x00000000 0x00030002 0x00050004
  )

  ;; Vectors with u16 boundary values (MIN=0, MAX=65535) in high lanes
  (func (export "get_boundary_test_vector1") (result v128)
    ;; u16x8: [0, 0, 0, 0, 65535, 65535, 65535, 65535] (high lanes 4-7: all MAX)
    ;; u16 MAX = 65535 = 0xFFFF
    ;; Packed as i32x4: [0x00000000, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF]
    v128.const i32x4 0x00000000 0x00000000 0xFFFFFFFF 0xFFFFFFFF
  )

  (func (export "get_boundary_test_vector2") (result v128)
    ;; u16x8: [0, 0, 0, 0, 65535, 1, 2, 0] (high lanes 4-7: 65535, 1, 2, 0)
    ;; Packed as i32x4: [0x00000000, 0x00000000, 0x0001FFFF, 0x00000002]
    v128.const i32x4 0x00000000 0x00000000 0x0001FFFF 0x00000002
  )

  ;; Zero vector for zero absorption tests (zeros in high lanes)
  (func (export "get_zero_high_vector") (result v128)
    ;; u16x8: [100, 200, 300, 400, 0, 0, 0, 0] (zeros in high lanes 4-7)
    ;; Lower lanes have non-zero values but should be ignored by high extmul
    ;; Packed as i32x4: [0x00C80064, 0x0190012C, 0x00000000, 0x00000000]
    v128.const i32x4 0x00C80064 0x0190012C 0x00000000 0x00000000
  )

  ;; Non-zero vector for zero absorption tests in high lanes
  (func (export "get_nonzero_high_vector") (result v128)
    ;; u16x8: [1000, 2000, 3000, 4000, 100, 200, 300, 400] (non-zero in high lanes 4-7)
    ;; Lower lanes ignored by high extmul
    ;; Packed as i32x4: [0x07D003E8, 0x0FA00BB8, 0x00C80064, 0x0190012C]
    v128.const i32x4 0x07D003E8 0x0FA00BB8 0x00C80064 0x0190012C
  )

  ;; Identity test vector for high lanes (value × 1 = value)
  (func (export "get_identity_vector_high") (result v128)
    ;; u16x8: [0, 0, 0, 0, 1, 1, 1, 1] (identity values in high lanes 4-7)
    ;; Packed as i32x4: [0x00000000, 0x00000000, 0x00010001, 0x00010001]
    v128.const i32x4 0x00000000 0x00000000 0x00010001 0x00010001
  )

  (func (export "get_identity_test_vector_high") (result v128)
    ;; u16x8: [0, 0, 0, 0, 1234, 5678, 0, 0] (test values for identity in high lanes 4-5)
    ;; Packed as i32x4: [0x00000000, 0x00000000, 0x162E04D2, 0x00000000]
    v128.const i32x4 0x00000000 0x00000000 0x162E04D2 0x00000000
  )

  ;; Power of 2 test vectors for high lanes
  (func (export "get_power_of_2_vector1") (result v128)
    ;; u16x8: [0, 0, 0, 0, 2, 4, 8, 16] (powers of 2 in high lanes 4-7)
    ;; Packed as i32x4: [0x00000000, 0x00000000, 0x00040002, 0x00100008]
    v128.const i32x4 0x00000000 0x00000000 0x00040002 0x00100008
  )

  (func (export "get_power_of_2_vector2") (result v128)
    ;; u16x8: [0, 0, 0, 0, 3, 5, 7, 9] (multipliers for power of 2 test)
    ;; Packed as i32x4: [0x00000000, 0x00000000, 0x00050003, 0x00090007]
    v128.const i32x4 0x00000000 0x00000000 0x00050003 0x00090007
  )

  ;; Maximum value multiplication test vectors
  (func (export "get_max_value_vector1") (result v128)
    ;; u16x8: [0, 0, 0, 0, 65535, 65535, 32768, 16384] (max and large values)
    ;; 32768 = 0x8000, 16384 = 0x4000
    ;; Packed as i32x4: [0x00000000, 0x00000000, 0xFFFFFFFF, 0x40008000]
    v128.const i32x4 0x00000000 0x00000000 0xFFFFFFFF 0x40008000
  )

  (func (export "get_max_value_vector2") (result v128)
    ;; u16x8: [0, 0, 0, 0, 65535, 2, 4, 8] (max with small multipliers)
    ;; Packed as i32x4: [0x00000000, 0x00000000, 0x0002FFFF, 0x00080004]
    v128.const i32x4 0x00000000 0x00000000 0x0002FFFF 0x00080004
  )

  ;; Validation test vectors for module loading
  (func (export "get_validation_vector1") (result v128)
    ;; u16x8: [0, 0, 0, 0, 100, 200, 300, 400] (simple validation values)
    ;; Packed as i32x4: [0x00000000, 0x00000000, 0x00C80064, 0x0190012C]
    v128.const i32x4 0x00000000 0x00000000 0x00C80064 0x0190012C
  )

  (func (export "get_validation_vector2") (result v128)
    ;; u16x8: [0, 0, 0, 0, 10, 15, 25, 50] (validation multipliers)
    ;; Packed as i32x4: [0x00000000, 0x00000000, 0x000F000A, 0x00320019]
    v128.const i32x4 0x00000000 0x00000000 0x000F000A 0x00320019
  )

  ;; Additional regression test vectors for edge cases in high lanes (unsigned)
  (func (export "get_regression_vector1") (result v128)
    ;; u16x8: [0, 0, 0, 0, 1000, 2000, 3000, 4000] (regression test values)
    ;; Packed as i32x4: [0x00000000, 0x00000000, 0x07D003E8, 0x0FA00BB8]
    v128.const i32x4 0x00000000 0x00000000 0x07D003E8 0x0FA00BB8
  )

  (func (export "get_regression_vector2") (result v128)
    ;; u16x8: [0, 0, 0, 0, 5, 10, 15, 20] (simple regression test values)
    ;; Packed as i32x4: [0x00000000, 0x00000000, 0x000A0005, 0x0014000F]
    v128.const i32x4 0x00000000 0x00000000 0x000A0005 0x0014000F
  )

  ;; Large number test vectors for comprehensive coverage
  (func (export "get_large_number_vector1") (result v128)
    ;; u16x8: [0, 0, 0, 0, 50000, 60000, 40000, 30000] (large unsigned values)
    ;; 50000 = 0xC350, 60000 = 0xEA60, 40000 = 0x9C40, 30000 = 0x7530
    ;; Packed as i32x4: [0x00000000, 0x00000000, 0xEA60C350, 0x75309C40]
    v128.const i32x4 0x00000000 0x00000000 0xEA60C350 0x75309C40
  )

  (func (export "get_large_number_vector2") (result v128)
    ;; u16x8: [0, 0, 0, 0, 2, 3, 4, 5] (small multipliers for large numbers)
    ;; Packed as i32x4: [0x00000000, 0x00000000, 0x00030002, 0x00050004]
    v128.const i32x4 0x00000000 0x00000000 0x00030002 0x00050004
  )
)