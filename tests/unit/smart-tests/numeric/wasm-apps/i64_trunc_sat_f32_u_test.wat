(module
  ;; Test module for i64.trunc_sat_f32_u opcode validation
  ;; Provides comprehensive test functions for saturating truncation from f32 to unsigned i64

  ;; Main test function for i64.trunc_sat_f32_u operation
  ;; @param f32 - Input f32 value to convert
  ;; @return i64 - Converted unsigned i64 value (saturated to [0, UINT64_MAX])
  (func (export "test_i64_trunc_sat_f32_u") (param f32) (result i64)
    local.get 0          ;; Load f32 parameter onto stack
    i64.trunc_sat_f32_u  ;; Convert f32 to unsigned i64 with saturation
  )

  ;; Auxiliary function to test basic conversion with multiple inputs
  ;; Demonstrates usage patterns and validates consistent behavior
  (func (export "test_basic_conversion") (param f32 f32 f32) (result i64 i64 i64)
    ;; Convert first parameter
    local.get 0
    i64.trunc_sat_f32_u

    ;; Convert second parameter
    local.get 1
    i64.trunc_sat_f32_u

    ;; Convert third parameter
    local.get 2
    i64.trunc_sat_f32_u
  )

  ;; Test function for boundary value validation
  ;; Tests overflow and underflow saturation behavior
  (func (export "test_boundary_saturation") (param f32 f32) (result i64 i64)
    ;; Test positive overflow case
    local.get 0
    i64.trunc_sat_f32_u

    ;; Test negative underflow case
    local.get 1
    i64.trunc_sat_f32_u
  )

  ;; Test function for special IEEE-754 float values
  ;; Validates NaN, infinity, and zero handling
  (func (export "test_special_values") (param f32 f32 f32 f32) (result i64 i64 i64 i64)
    ;; Test NaN conversion
    local.get 0
    i64.trunc_sat_f32_u

    ;; Test positive infinity
    local.get 1
    i64.trunc_sat_f32_u

    ;; Test negative infinity
    local.get 2
    i64.trunc_sat_f32_u

    ;; Test zero value
    local.get 3
    i64.trunc_sat_f32_u
  )

  ;; Stress test function for comprehensive validation
  ;; Tests multiple edge cases in sequence for performance validation
  (func (export "test_comprehensive_validation") (param f32) (result i64)
    local.get 0
    i64.trunc_sat_f32_u

    ;; Additional validation operations can be chained here
    ;; Currently returns single conversion result
  )

  ;; Memory allocation for potential future extensions
  (memory 1)
)