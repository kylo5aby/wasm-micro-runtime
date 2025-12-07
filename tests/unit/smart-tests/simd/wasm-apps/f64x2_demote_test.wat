(module
  ;; Test module for f32x4.demote_f64x2_zero SIMD instruction validation
  ;; Provides comprehensive testing functions for f64→f32 conversion scenarios

  ;; Import memory for storing and loading v128 data
  (memory (export "memory") 1)

  ;; Function to test f32x4.demote_f64x2_zero with two f64 inputs
  ;; Parameters: f64 value1, f64 value2
  ;; Returns: v128 containing two f32 results (in lanes 0 and 1)
  (func (export "test_f64x2_demote") (param $f64_1 f64) (param $f64_2 f64) (result v128)
    ;; Store f64 values in memory to create f64x2 vector
    (i32.const 0)       ;; Memory offset for first f64
    local.get $f64_1
    f64.store

    (i32.const 8)       ;; Memory offset for second f64
    local.get $f64_2
    f64.store

    ;; Load as v128 f64x2 vector and apply demote
    (i32.const 0)
    v128.load
    f32x4.demote_f64x2_zero

    ;; Return the resulting f32x4 vector
  )

  ;; Helper function to test individual lane extraction (for debugging)
  (func (export "test_f64x2_demote_lane0") (param $f64_1 f64) (param $f64_2 f64) (result f32)
    ;; Store f64 values in memory
    (i32.const 16)
    local.get $f64_1
    f64.store

    (i32.const 24)
    local.get $f64_2
    f64.store

    ;; Load and demote, then extract lane 0
    (i32.const 16)
    v128.load
    f32x4.demote_f64x2_zero
    f32x4.extract_lane 0
  )

  (func (export "test_f64x2_demote_lane1") (param $f64_1 f64) (param $f64_2 f64) (result f32)
    ;; Store f64 values in memory
    (i32.const 32)
    local.get $f64_1
    f64.store

    (i32.const 40)
    local.get $f64_2
    f64.store

    ;; Load and demote, then extract lane 1
    (i32.const 32)
    v128.load
    f32x4.demote_f64x2_zero
    f32x4.extract_lane 1
  )
)