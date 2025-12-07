(module
  ;; Test function 1: Basic extended multiplication for high lanes
  ;; Takes two v128 parameters and returns v128 result
  ;; Performs i32x4.extmul_high_i16x8_s on the inputs (operates on lanes 4-7)
  (func (export "test_basic_extmul") (param v128 v128) (result v128)
    local.get 0  ;; Load first v128 parameter (i16x8)
    local.get 1  ;; Load second v128 parameter (i16x8)
    i32x4.extmul_high_i16x8_s  ;; Perform extended multiplication on higher 4 lanes (4-7)
  )

  ;; Test function 2: Signed arithmetic test for high lanes
  ;; Tests mixed positive/negative values in high lanes
  (func (export "test_signed_extmul") (param v128 v128) (result v128)
    local.get 0
    local.get 1
    i32x4.extmul_high_i16x8_s
  )

  ;; Test function 3: Boundary values test for high lanes
  ;; Tests i16 MIN/MAX boundary conditions in high lanes
  (func (export "test_boundary_extmul") (param v128 v128) (result v128)
    local.get 0
    local.get 1
    i32x4.extmul_high_i16x8_s
  )

  ;; Test function 4: Zero absorption test for high lanes
  ;; Tests zero multiplication property (0 × n = 0) in high lanes
  (func (export "test_zero_extmul") (param v128 v128) (result v128)
    local.get 0
    local.get 1
    i32x4.extmul_high_i16x8_s
  )

  ;; Test function 5: Commutative property test for high lanes
  ;; Tests commutative multiplication (a × b = b × a) in high lanes
  (func (export "test_commutative_extmul") (param v128 v128) (result v128)
    local.get 0
    local.get 1
    i32x4.extmul_high_i16x8_s
  )

  ;; Test function 6: Comprehensive test with various inputs in high lanes
  ;; Tests multiple scenarios in a single function
  (func (export "test_comprehensive_extmul") (param v128 v128) (result v128)
    local.get 0
    local.get 1
    i32x4.extmul_high_i16x8_s
  )

  ;; Test function 7: Lane independence test for high lanes
  ;; Verifies that each high lane is computed independently
  (func (export "test_lane_independence") (param v128 v128) (result v128)
    local.get 0
    local.get 1
    i32x4.extmul_high_i16x8_s
  )

  ;; Test function 8: Specific value test for regression testing in high lanes
  ;; Tests known good values for consistent behavior verification
  (func (export "test_regression_values") (param v128 v128) (result v128)
    local.get 0
    local.get 1
    i32x4.extmul_high_i16x8_s
  )

  ;; Test function 9: Performance baseline test for high lanes
  ;; Simple test for execution timing baseline
  (func (export "test_performance_baseline") (param v128 v128) (result v128)
    local.get 0
    local.get 1
    i32x4.extmul_high_i16x8_s
  )

  ;; Test function 10: Cross-mode consistency test for high lanes
  ;; Ensures identical results between interpreter and AOT modes
  (func (export "test_cross_mode_consistency") (param v128 v128) (result v128)
    local.get 0
    local.get 1
    i32x4.extmul_high_i16x8_s
  )

  ;; Test constant vectors for specific test cases (focusing on high lanes 4-7)
  ;; Vector with small positive values for basic testing in high lanes
  (func (export "get_basic_test_vector1") (result v128)
    ;; i16x8: [0, 0, 0, 0, 5, 6, 7, 8] (high lanes 4-7: 5, 6, 7, 8)
    ;; Packed as i32x4: [0x00000000, 0x00000000, 0x00060005, 0x00080007]
    v128.const i32x4 0x00000000 0x00000000 0x00060005 0x00080007
  )

  (func (export "get_basic_test_vector2") (result v128)
    ;; i16x8: [0, 0, 0, 0, 14, 15, 16, 17] (high lanes 4-7: 14, 15, 16, 17)
    ;; Packed as i32x4: [0x00000000, 0x00000000, 0x000F000E, 0x00110010]
    v128.const i32x4 0x00000000 0x00000000 0x000F000E 0x00110010
  )

  ;; Vector with signed values for signed arithmetic testing in high lanes
  (func (export "get_signed_test_vector1") (result v128)
    ;; i16x8: [0, 0, 0, 0, -1, -2, -3, -4] (high lanes 4-7: -1, -2, -3, -4)
    ;; -1 = 0xFFFF, -2 = 0xFFFE, -3 = 0xFFFD, -4 = 0xFFFC
    ;; Packed as i32x4: [0x00000000, 0x00000000, 0xFFFEFFFF, 0xFFFCFFFD]
    v128.const i32x4 0x00000000 0x00000000 0xFFFEFFFF 0xFFFCFFFD
  )

  (func (export "get_signed_test_vector2") (result v128)
    ;; i16x8: [0, 0, 0, 0, 5, -6, 7, -8] (high lanes 4-7: 5, -6, 7, -8)
    ;; -6 = 0xFFFA, -8 = 0xFFF8
    ;; Packed as i32x4: [0x00000000, 0x00000000, 0xFFFA0005, 0xFFF80007]
    v128.const i32x4 0x00000000 0x00000000 0xFFFA0005 0xFFF80007
  )

  ;; Vectors with boundary values (i16 MIN/MAX) in high lanes
  (func (export "get_boundary_test_vector1") (result v128)
    ;; i16x8: [0, 0, 0, 0, 32767, -32768, 32767, -32768] (high lanes 4-7)
    ;; i16 MAX = 32767 = 0x7FFF, i16 MIN = -32768 = 0x8000
    ;; Packed as i32x4: [0x00000000, 0x00000000, 0x80007FFF, 0x80007FFF]
    v128.const i32x4 0x00000000 0x00000000 0x80007FFF 0x80007FFF
  )

  (func (export "get_boundary_test_vector2") (result v128)
    ;; i16x8: [0, 0, 0, 0, 32767, 32767, -32768, -32768] (high lanes 4-7)
    ;; Packed as i32x4: [0x00000000, 0x00000000, 0x7FFF7FFF, 0x80008000]
    v128.const i32x4 0x00000000 0x00000000 0x7FFF7FFF 0x80008000
  )

  ;; Zero vector for zero absorption tests (zeros in high lanes)
  (func (export "get_zero_high_vector") (result v128)
    ;; i16x8: [100, 200, 300, 400, 0, 0, 0, 0] (zeros in high lanes 4-7)
    ;; Lower lanes have non-zero values but should be ignored by high extmul
    ;; Packed as i32x4: [0x00C80064, 0x0190012C, 0x00000000, 0x00000000]
    v128.const i32x4 0x00C80064 0x0190012C 0x00000000 0x00000000
  )

  ;; Non-zero vector for zero absorption tests in high lanes
  (func (export "get_nonzero_high_vector") (result v128)
    ;; i16x8: [1000, 2000, 3000, 4000, 100, 200, 300, 400] (non-zero in high lanes 4-7)
    ;; Lower lanes ignored by high extmul
    ;; Packed as i32x4: [0x07D003E8, 0x0FA00BB8, 0x00C80064, 0x0190012C]
    v128.const i32x4 0x07D003E8 0x0FA00BB8 0x00C80064 0x0190012C
  )

  ;; Commutative test vectors for high lanes
  (func (export "get_commutative_test_vectorA") (result v128)
    ;; i16x8: [0, 0, 0, 0, 15, -20, 25, -30] (high lanes 4-7: 15, -20, 25, -30)
    ;; -20 = 0xFFEC, -30 = 0xFFE2
    ;; Packed as i32x4: [0x00000000, 0x00000000, 0xFFEC000F, 0xFFE20019]
    v128.const i32x4 0x00000000 0x00000000 0xFFEC000F 0xFFE20019
  )

  (func (export "get_commutative_test_vectorB") (result v128)
    ;; i16x8: [0, 0, 0, 0, 7, 8, -9, 10] (high lanes 4-7: 7, 8, -9, 10)
    ;; -9 = 0xFFF7
    ;; Packed as i32x4: [0x00000000, 0x00000000, 0x00080007, 0x000AFFF7]
    v128.const i32x4 0x00000000 0x00000000 0x00080007 0x000AFFF7
  )

  ;; Identity test helper functions for high lanes
  (func (export "get_identity_vector_high") (result v128)
    ;; i16x8: [0, 0, 0, 0, 1, 1, 1, 1] (identity values in high lanes 4-7)
    ;; Packed as i32x4: [0x00000000, 0x00000000, 0x00010001, 0x00010001]
    v128.const i32x4 0x00000000 0x00000000 0x00010001 0x00010001
  )

  (func (export "get_identity_test_vector_high") (result v128)
    ;; i16x8: [0, 0, 0, 0, 100, 200, 300, 400] (test values in high lanes 4-7)
    ;; Packed as i32x4: [0x00000000, 0x00000000, 0x00C80064, 0x0190012C]
    v128.const i32x4 0x00000000 0x00000000 0x00C80064 0x0190012C
  )

  ;; Additional regression test vectors for edge cases in high lanes
  (func (export "get_regression_vector1") (result v128)
    ;; i16x8: [0, 0, 0, 0, 1000, -1500, 2000, -2500] (regression test values)
    ;; -1500 = 0xFA24, -2500 = 0xF63C
    ;; Packed as i32x4: [0x00000000, 0x00000000, 0xFA2403E8, 0xF63C07D0]
    v128.const i32x4 0x00000000 0x00000000 0xFA2403E8 0xF63C07D0
  )

  (func (export "get_regression_vector2") (result v128)
    ;; i16x8: [0, 0, 0, 0, 3, -4, 5, -6] (simple regression test values)
    ;; -4 = 0xFFFC, -6 = 0xFFFA
    ;; Packed as i32x4: [0x00000000, 0x00000000, 0xFFFC0003, 0xFFFA0005]
    v128.const i32x4 0x00000000 0x00000000 0xFFFC0003 0xFFFA0005
  )
)