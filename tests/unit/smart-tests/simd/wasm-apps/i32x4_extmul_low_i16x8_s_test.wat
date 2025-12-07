(module
  ;; Test function 1: Basic extended multiplication
  ;; Takes two v128 parameters and returns v128 result
  ;; Performs i32x4.extmul_low_i16x8_s on the inputs
  (func (export "test_basic_extmul") (param v128 v128) (result v128)
    local.get 0  ;; Load first v128 parameter (i16x8)
    local.get 1  ;; Load second v128 parameter (i16x8)
    i32x4.extmul_low_i16x8_s  ;; Perform extended multiplication on lower 4 lanes
  )

  ;; Test function 2: Signed arithmetic test
  ;; Tests mixed positive/negative values
  (func (export "test_signed_extmul") (param v128 v128) (result v128)
    local.get 0
    local.get 1
    i32x4.extmul_low_i16x8_s
  )

  ;; Test function 3: Boundary values test
  ;; Tests i16 MIN/MAX boundary conditions
  (func (export "test_boundary_extmul") (param v128 v128) (result v128)
    local.get 0
    local.get 1
    i32x4.extmul_low_i16x8_s
  )

  ;; Test function 4: Zero absorption test
  ;; Tests zero multiplication property (0 × n = 0)
  (func (export "test_zero_extmul") (param v128 v128) (result v128)
    local.get 0
    local.get 1
    i32x4.extmul_low_i16x8_s
  )

  ;; Test function 5: Identity property test
  ;; Tests identity multiplication (n × 1 = n)
  (func (export "test_identity_extmul") (param v128 v128) (result v128)
    local.get 0
    local.get 1
    i32x4.extmul_low_i16x8_s
  )

  ;; Test function 6: Comprehensive test with various inputs
  ;; Tests multiple scenarios in a single function
  (func (export "test_comprehensive_extmul") (param v128 v128) (result v128)
    local.get 0
    local.get 1
    i32x4.extmul_low_i16x8_s
  )

  ;; Test function 7: Lane independence test
  ;; Verifies that each lane is computed independently
  (func (export "test_lane_independence") (param v128 v128) (result v128)
    local.get 0
    local.get 1
    i32x4.extmul_low_i16x8_s
  )

  ;; Test function 8: Specific value test for regression testing
  ;; Tests known good values for consistent behavior verification
  (func (export "test_regression_values") (param v128 v128) (result v128)
    local.get 0
    local.get 1
    i32x4.extmul_low_i16x8_s
  )

  ;; Test function 9: Performance baseline test
  ;; Simple test for execution timing baseline
  (func (export "test_performance_baseline") (param v128 v128) (result v128)
    local.get 0
    local.get 1
    i32x4.extmul_low_i16x8_s
  )

  ;; Test function 10: Cross-mode consistency test
  ;; Ensures identical results between interpreter and AOT modes
  (func (export "test_cross_mode_consistency") (param v128 v128) (result v128)
    local.get 0
    local.get 1
    i32x4.extmul_low_i16x8_s
  )

  ;; Test constant vectors for specific test cases
  ;; Vector with small positive values for basic testing
  (func (export "get_basic_test_vector1") (result v128)
    ;; i16x8: [10, 20, 30, 40, 0, 0, 0, 0]
    ;; Packed as i32x4: [0x0014000A, 0x0028001E, 0x00000000, 0x00000000]
    v128.const i32x4 0x0014000A 0x0028001E 0x00000000 0x00000000
  )

  (func (export "get_basic_test_vector2") (result v128)
    ;; i16x8: [2, 3, 4, 5, 0, 0, 0, 0]
    ;; Packed as i32x4: [0x00030002, 0x00050004, 0x00000000, 0x00000000]
    v128.const i32x4 0x00030002 0x00050004 0x00000000 0x00000000
  )

  ;; Vector with signed values for signed arithmetic testing
  (func (export "get_signed_test_vector1") (result v128)
    ;; i16x8: [100, -200, 300, -400, 0, 0, 0, 0]
    ;; Packed as i32x4: [0xFF380064, 0xFE70012C, 0x00000000, 0x00000000]
    v128.const i32x4 0xFF380064 0xFE70012C 0x00000000 0x00000000
  )

  (func (export "get_signed_test_vector2") (result v128)
    ;; i16x8: [-2, 3, -4, 5, 0, 0, 0, 0]
    ;; Packed as i32x4: [0x0003FFFE, 0x0005FFFC, 0x00000000, 0x00000000]
    v128.const i32x4 0x0003FFFE 0x0005FFFC 0x00000000 0x00000000
  )

  ;; Vectors with boundary values (i16 MIN/MAX)
  (func (export "get_boundary_test_vector1") (result v128)
    ;; i16x8: [32767, -32768, 32767, -32768, 0, 0, 0, 0]
    ;; i16 MAX = 32767 = 0x7FFF, i16 MIN = -32768 = 0x8000
    ;; Packed as i32x4: [0x80007FFF, 0x80007FFF, 0x00000000, 0x00000000]
    v128.const i32x4 0x80007FFF 0x80007FFF 0x00000000 0x00000000
  )

  (func (export "get_boundary_test_vector2") (result v128)
    ;; i16x8: [32767, 32767, -32768, -32768, 0, 0, 0, 0]
    ;; Packed as i32x4: [0x7FFF7FFF, 0x80008000, 0x00000000, 0x00000000]
    v128.const i32x4 0x7FFF7FFF 0x80008000 0x00000000 0x00000000
  )

  ;; Zero vector for zero absorption tests
  (func (export "get_zero_vector") (result v128)
    ;; i16x8: [0, 0, 0, 0, 0, 0, 0, 0]
    v128.const i32x4 0x00000000 0x00000000 0x00000000 0x00000000
  )

  ;; Identity vector for identity property tests
  (func (export "get_identity_vector") (result v128)
    ;; i16x8: [1, 1, 1, 1, 0, 0, 0, 0]
    ;; Packed as i32x4: [0x00010001, 0x00010001, 0x00000000, 0x00000000]
    v128.const i32x4 0x00010001 0x00010001 0x00000000 0x00000000
  )

  ;; Test vector for identity property testing
  (func (export "get_identity_test_vector") (result v128)
    ;; i16x8: [100, 200, 300, 400, 0, 0, 0, 0]
    ;; Packed as i32x4: [0x00C80064, 0x0190012C, 0x00000000, 0x00000000]
    v128.const i32x4 0x00C80064 0x0190012C 0x00000000 0x00000000
  )
)