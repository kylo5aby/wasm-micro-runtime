(module
  ;; Test function 1: Basic unsigned extended multiplication
  ;; Takes two v128 parameters and returns v128 result
  ;; Performs i32x4.extmul_low_i16x8_u on the inputs
  (func (export "test_basic_extmul") (param v128 v128) (result v128)
    local.get 0  ;; Load first v128 parameter (i16x8)
    local.get 1  ;; Load second v128 parameter (i16x8)
    i32x4.extmul_low_i16x8_u  ;; Perform unsigned extended multiplication on lower 4 lanes
  )

  ;; Test function 2: Unsigned arithmetic test
  ;; Tests large unsigned values (that would be negative in signed interpretation)
  (func (export "test_unsigned_extmul") (param v128 v128) (result v128)
    local.get 0
    local.get 1
    i32x4.extmul_low_i16x8_u
  )

  ;; Test function 3: Boundary values test
  ;; Tests unsigned i16 MAX (65535) boundary conditions
  (func (export "test_boundary_extmul") (param v128 v128) (result v128)
    local.get 0
    local.get 1
    i32x4.extmul_low_i16x8_u
  )

  ;; Test function 4: Zero absorption test
  ;; Tests zero multiplication property (0 × n = 0)
  (func (export "test_zero_extmul") (param v128 v128) (result v128)
    local.get 0
    local.get 1
    i32x4.extmul_low_i16x8_u
  )

  ;; Test function 5: Identity property test
  ;; Tests identity multiplication (n × 1 = n)
  (func (export "test_identity_extmul") (param v128 v128) (result v128)
    local.get 0
    local.get 1
    i32x4.extmul_low_i16x8_u
  )

  ;; Test function 6: Comprehensive test with various unsigned inputs
  ;; Tests multiple scenarios in a single function
  (func (export "test_comprehensive_extmul") (param v128 v128) (result v128)
    local.get 0
    local.get 1
    i32x4.extmul_low_i16x8_u
  )

  ;; Test function 7: Lane independence test
  ;; Verifies that each lane is computed independently
  (func (export "test_lane_independence") (param v128 v128) (result v128)
    local.get 0
    local.get 1
    i32x4.extmul_low_i16x8_u
  )

  ;; Test function 8: Specific unsigned value test for regression testing
  ;; Tests known good unsigned values for consistent behavior verification
  (func (export "test_regression_values") (param v128 v128) (result v128)
    local.get 0
    local.get 1
    i32x4.extmul_low_i16x8_u
  )

  ;; Test function 9: Performance baseline test
  ;; Simple test for execution timing baseline
  (func (export "test_performance_baseline") (param v128 v128) (result v128)
    local.get 0
    local.get 1
    i32x4.extmul_low_i16x8_u
  )

  ;; Test function 10: Cross-mode consistency test
  ;; Ensures identical results between interpreter and AOT modes
  (func (export "test_cross_mode_consistency") (param v128 v128) (result v128)
    local.get 0
    local.get 1
    i32x4.extmul_low_i16x8_u
  )

  ;; Test constant vectors for specific unsigned test cases
  ;; Vector with small unsigned positive values for basic testing
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

  ;; Vector with large unsigned values (previously negative in signed interpretation)
  (func (export "get_unsigned_test_vector1") (result v128)
    ;; i16x8: [32768, 40000, 50000, 60000, 0, 0, 0, 0]
    ;; 32768 = 0x8000, 40000 = 0x9C40, 50000 = 0xC350, 60000 = 0xEA60
    ;; Packed as i32x4: [0x9C408000, 0xEA60C350, 0x00000000, 0x00000000]
    v128.const i32x4 0x9C408000 0xEA60C350 0x00000000 0x00000000
  )

  (func (export "get_unsigned_test_vector2") (result v128)
    ;; i16x8: [32768, 1000, 800, 600, 0, 0, 0, 0]
    ;; 32768 = 0x8000, 1000 = 0x03E8, 800 = 0x0320, 600 = 0x0258
    ;; Packed as i32x4: [0x03E88000, 0x02580320, 0x00000000, 0x00000000]
    v128.const i32x4 0x03E88000 0x02580320 0x00000000 0x00000000
  )

  ;; Vectors with unsigned boundary values (u16 MAX = 65535)
  (func (export "get_boundary_test_vector1") (result v128)
    ;; i16x8: [65535, 65535, 65535, 65535, 0, 0, 0, 0]
    ;; u16 MAX = 65535 = 0xFFFF
    ;; Packed as i32x4: [0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0x00000000]
    v128.const i32x4 0xFFFFFFFF 0xFFFFFFFF 0x00000000 0x00000000
  )

  (func (export "get_boundary_test_vector2") (result v128)
    ;; i16x8: [65535, 32768, 1, 2, 0, 0, 0, 0]
    ;; 65535 = 0xFFFF, 32768 = 0x8000, 1 = 0x0001, 2 = 0x0002
    ;; Packed as i32x4: [0x8000FFFF, 0x00020001, 0x00000000, 0x00000000]
    v128.const i32x4 0x8000FFFF 0x00020001 0x00000000 0x00000000
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
    ;; 100 = 0x0064, 200 = 0x00C8, 300 = 0x012C, 400 = 0x0190
    ;; Packed as i32x4: [0x00C80064, 0x0190012C, 0x00000000, 0x00000000]
    v128.const i32x4 0x00C80064 0x0190012C 0x00000000 0x00000000
  )

  ;; Test vector for maximum unsigned multiplication validation
  (func (export "get_max_unsigned_vector1") (result v128)
    ;; i16x8: [65535, 65000, 64000, 63000, 0, 0, 0, 0]
    ;; 65535 = 0xFFFF, 65000 = 0xFDE8, 64000 = 0xFA00, 63000 = 0xF618
    ;; Packed as i32x4: [0xFDE8FFFF, 0xF618FA00, 0x00000000, 0x00000000]
    v128.const i32x4 0xFDE8FFFF 0xF618FA00 0x00000000 0x00000000
  )

  (func (export "get_max_unsigned_vector2") (result v128)
    ;; i16x8: [1000, 800, 600, 500, 0, 0, 0, 0]
    ;; 1000 = 0x03E8, 800 = 0x0320, 600 = 0x0258, 500 = 0x01F4
    ;; Packed as i32x4: [0x032003E8, 0x01F40258, 0x00000000, 0x00000000]
    v128.const i32x4 0x032003E8 0x01F40258 0x00000000 0x00000000
  )
)