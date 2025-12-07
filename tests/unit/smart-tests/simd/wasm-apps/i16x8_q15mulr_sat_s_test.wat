(module
  ;; Import memory for potential data storage if needed
  (memory $mem 1 1)

  ;; Test function for i16x8.q15mulr_sat_s opcode
  ;; Takes two v128 parameters and returns v128 result
  ;; Function signature: (v128, v128) -> v128
  (func $i16x8_q15mulr_sat_s_test (param $a v128) (param $b v128) (result v128)
    ;; Load operands onto stack
    local.get $a
    local.get $b

    ;; Execute i16x8.q15mulr_sat_s operation
    ;; This performs Q15 saturating multiplication with rounding
    ;; For each lane i: result[i] = saturate(round((a[i] * b[i]) >> 15))
    i16x8.q15mulr_sat_s

    ;; Return the result v128
  )

  ;; Export the test function for C++ test harness
  (export "i16x8_q15mulr_sat_s_test" (func $i16x8_q15mulr_sat_s_test))

  ;; Additional test functions for specific scenarios

  ;; Test function for basic Q15 multiplication scenarios
  (func $test_basic_q15_mul (param $a v128) (param $b v128) (result v128)
    local.get $a
    local.get $b
    i16x8.q15mulr_sat_s
  )
  (export "test_basic_q15_mul" (func $test_basic_q15_mul))

  ;; Test function for saturation boundary cases
  (func $test_saturation_boundaries (param $a v128) (param $b v128) (result v128)
    local.get $a
    local.get $b
    i16x8.q15mulr_sat_s
  )
  (export "test_saturation_boundaries" (func $test_saturation_boundaries))

  ;; Test function for zero operand scenarios
  (func $test_zero_operations (param $a v128) (param $b v128) (result v128)
    local.get $a
    local.get $b
    i16x8.q15mulr_sat_s
  )
  (export "test_zero_operations" (func $test_zero_operations))

  ;; Test function for extreme value combinations
  (func $test_extreme_values (param $a v128) (param $b v128) (result v128)
    local.get $a
    local.get $b
    i16x8.q15mulr_sat_s
  )
  (export "test_extreme_values" (func $test_extreme_values))

  ;; Test function for rounding behavior validation
  (func $test_rounding_behavior (param $a v128) (param $b v128) (result v128)
    local.get $a
    local.get $b
    i16x8.q15mulr_sat_s
  )
  (export "test_rounding_behavior" (func $test_rounding_behavior))

  ;; Test function for sign handling verification
  (func $test_mixed_sign_multiplication (param $a v128) (param $b v128) (result v128)
    local.get $a
    local.get $b
    i16x8.q15mulr_sat_s
  )
  (export "test_mixed_sign_multiplication" (func $test_mixed_sign_multiplication))

  ;; Comprehensive test function with predefined test vectors
  (func $test_comprehensive_q15_scenarios (result v128)
    ;; Test case 1: Basic Q15 multiplication
    ;; a = [0.5, -0.25, 0.75, -0.5, 0.125, -0.375, 0.625, 0.0] in Q15 format
    ;; b = [0.5, 0.5, 0.25, -0.25, ~1.0, 0.5, 0.375, ~1.0] in Q15 format
    v128.const i16x8 16384 -8192 24576 -16384 4096 -12288 20480 0
    v128.const i16x8 16384 16384 8192 -8192 32767 16384 12288 32767
    i16x8.q15mulr_sat_s
  )
  (export "test_comprehensive_q15_scenarios" (func $test_comprehensive_q15_scenarios))

  ;; Test function for saturation edge cases
  (func $test_saturation_edge_cases (result v128)
    ;; Test vectors that would cause overflow without saturation
    ;; Large positive and negative values that test saturation boundaries
    v128.const i16x8 32767 -32768 32767 -32768 30000 -30000 25000 -25000
    v128.const i16x8 32767 -32768 -32768 32767 32000 32000 -32000 -32000
    i16x8.q15mulr_sat_s
  )
  (export "test_saturation_edge_cases" (func $test_saturation_edge_cases))

  ;; Test function for mathematical properties validation
  (func $test_mathematical_properties (result v128)
    ;; Test commutative property: A × B should equal B × A
    ;; Using specific values to verify mathematical correctness
    v128.const i16x8 8192 -4096 16384 -8192 12288 -6144 20480 -10240
    v128.const i16x8 4096 8192 -8192 16384 6144 12288 -10240 20480
    i16x8.q15mulr_sat_s
  )
  (export "test_mathematical_properties" (func $test_mathematical_properties))

  ;; Test function for Q15 format validation
  (func $test_q15_format_compliance (result v128)
    ;; Validate Q15 fixed-point format compliance
    ;; Test values representing common Q15 fractions
    v128.const i16x8 16384 8192 4096 2048 1024 512 256 128    ;; 0.5, 0.25, 0.125, etc.
    v128.const i16x8 32767 16384 8192 4096 2048 1024 512 256  ;; ~1.0, 0.5, 0.25, etc.
    i16x8.q15mulr_sat_s
  )
  (export "test_q15_format_compliance" (func $test_q15_format_compliance))

  ;; Test function for lane independence verification
  (func $test_lane_independence (result v128)
    ;; Each lane uses different value combinations to verify independent processing
    ;; Mixed patterns to ensure no cross-lane interference
    v128.const i16x8 1 -1 32767 -32768 16384 -16384 8192 -8192
    v128.const i16x8 32767 32767 1 1 2 2 4 4
    i16x8.q15mulr_sat_s
  )
  (export "test_lane_independence" (func $test_lane_independence))

  ;; Memory-based test function for complex scenarios
  (func $test_memory_based_operations
    ;; Store test vectors in memory and load them for operations
    ;; This tests the integration with memory operations

    ;; Store first test vector at memory offset 0
    i32.const 0
    v128.const i16x8 12345 -6789 23456 -12345 7890 -23456 15432 -7890
    v128.store

    ;; Store second test vector at memory offset 16
    i32.const 16
    v128.const i16x8 9876 12345 -9876 6789 -12345 9876 6789 -12345
    v128.store

    ;; Load vectors, perform Q15 multiplication and store result
    (v128.store (i32.const 32)
      (i16x8.q15mulr_sat_s
        (v128.load (i32.const 0))
        (v128.load (i32.const 16))))
  )
  (export "test_memory_based_operations" (func $test_memory_based_operations))

  ;; Test function for performance benchmarking
  (func $test_performance_benchmark (param $iterations i32) (result v128)
    (local $i i32)
    (local $result v128)

    ;; Initialize test vectors
    v128.const i16x8 16384 -8192 24576 -16384 4096 -12288 20480 0
    local.set $result

    ;; Loop for performance testing
    loop $loop
      local.get $result
      v128.const i16x8 16384 16384 8192 -8192 32767 16384 12288 32767
      i16x8.q15mulr_sat_s
      local.set $result

      local.get $i
      i32.const 1
      i32.add
      local.tee $i
      local.get $iterations
      i32.lt_s
      br_if $loop
    end

    local.get $result
  )
  (export "test_performance_benchmark" (func $test_performance_benchmark))

)