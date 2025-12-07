(module
  ;; Test module for f64x2.promote_low_f32x4 SIMD instruction validation
  ;; Provides comprehensive testing functions for f32→f64 promotion scenarios

  ;; Import memory for storing and loading v128 data
  (memory (export "memory") 1)

  ;; Function to test f64x2.promote_low_f32x4 with basic values
  ;; Parameters: f32 lane0, f32 lane1, f32 lane2, f32 lane3
  ;; Returns: v128 containing two f64 results promoted from lanes 0 and 1
  (func (export "test_promote_basic") (param $f32_0 f32) (param $f32_1 f32) (param $f32_2 f32) (param $f32_3 f32) (result v128)
    ;; Store f32 values in memory to create f32x4 vector
    (i32.const 0)       ;; Memory offset for f32x4 vector
    local.get $f32_0    ;; Lane 0 (will be promoted)
    f32.store

    (i32.const 4)       ;; Memory offset for lane 1
    local.get $f32_1    ;; Lane 1 (will be promoted)
    f32.store

    (i32.const 8)       ;; Memory offset for lane 2
    local.get $f32_2    ;; Lane 2 (ignored during promotion)
    f32.store

    (i32.const 12)      ;; Memory offset for lane 3
    local.get $f32_3    ;; Lane 3 (ignored during promotion)
    f32.store

    ;; Load as v128 f32x4 vector and apply promote_low
    (i32.const 0)
    v128.load
    f64x2.promote_low_f32x4

    ;; Return the resulting f64x2 vector
  )

  ;; Function to test f64x2.promote_low_f32x4 with boundary values
  ;; Parameters: f32 lane0, f32 lane1, f32 lane2, f32 lane3
  ;; Returns: v128 containing two f64 results promoted from lanes 0 and 1
  (func (export "test_promote_boundary") (param $f32_0 f32) (param $f32_1 f32) (param $f32_2 f32) (param $f32_3 f32) (result v128)
    ;; Store f32 values in memory to create f32x4 vector
    (i32.const 16)      ;; Use different memory offset to avoid interference
    local.get $f32_0
    f32.store

    (i32.const 20)
    local.get $f32_1
    f32.store

    (i32.const 24)
    local.get $f32_2
    f32.store

    (i32.const 28)
    local.get $f32_3
    f32.store

    ;; Load as v128 f32x4 vector and apply promote_low
    (i32.const 16)
    v128.load
    f64x2.promote_low_f32x4
  )

  ;; Function to test f64x2.promote_low_f32x4 with special values (NaN, infinity)
  ;; Parameters: f32 lane0, f32 lane1, f32 lane2, f32 lane3
  ;; Returns: v128 containing two f64 results promoted from lanes 0 and 1
  (func (export "test_promote_special") (param $f32_0 f32) (param $f32_1 f32) (param $f32_2 f32) (param $f32_3 f32) (result v128)
    ;; Store f32 values in memory to create f32x4 vector
    (i32.const 32)      ;; Use different memory offset
    local.get $f32_0
    f32.store

    (i32.const 36)
    local.get $f32_1
    f32.store

    (i32.const 40)
    local.get $f32_2
    f32.store

    (i32.const 44)
    local.get $f32_3
    f32.store

    ;; Load as v128 f32x4 vector and apply promote_low
    (i32.const 32)
    v128.load
    f64x2.promote_low_f32x4
  )

  ;; Function to test f64x2.promote_low_f32x4 with signed zero values
  ;; Parameters: f32 lane0, f32 lane1, f32 lane2, f32 lane3
  ;; Returns: v128 containing two f64 results promoted from lanes 0 and 1
  (func (export "test_promote_zero") (param $f32_0 f32) (param $f32_1 f32) (param $f32_2 f32) (param $f32_3 f32) (result v128)
    ;; Store f32 values in memory to create f32x4 vector
    (i32.const 48)      ;; Use different memory offset
    local.get $f32_0
    f32.store

    (i32.const 52)
    local.get $f32_1
    f32.store

    (i32.const 56)
    local.get $f32_2
    f32.store

    (i32.const 60)
    local.get $f32_3
    f32.store

    ;; Load as v128 f32x4 vector and apply promote_low
    (i32.const 48)
    v128.load
    f64x2.promote_low_f32x4
  )

  ;; Helper function to test individual lane extraction (for debugging)
  ;; Extracts the first promoted f64 value (from original f32 lane 0)
  (func (export "test_promote_extract_0") (param $f32_0 f32) (param $f32_1 f32) (param $f32_2 f32) (param $f32_3 f32) (result f64)
    ;; Store f32 values in memory
    (i32.const 64)
    local.get $f32_0
    f32.store

    (i32.const 68)
    local.get $f32_1
    f32.store

    (i32.const 72)
    local.get $f32_2
    f32.store

    (i32.const 76)
    local.get $f32_3
    f32.store

    ;; Load f32x4, promote to f64x2, and extract lane 0
    (i32.const 64)
    v128.load
    f64x2.promote_low_f32x4
    f64x2.extract_lane 0
  )

  ;; Helper function to extract the second promoted f64 value (from original f32 lane 1)
  (func (export "test_promote_extract_1") (param $f32_0 f32) (param $f32_1 f32) (param $f32_2 f32) (param $f32_3 f32) (result f64)
    ;; Store f32 values in memory
    (i32.const 80)
    local.get $f32_0
    f32.store

    (i32.const 84)
    local.get $f32_1
    f32.store

    (i32.const 88)
    local.get $f32_2
    f32.store

    (i32.const 92)
    local.get $f32_3
    f32.store

    ;; Load f32x4, promote to f64x2, and extract lane 1
    (i32.const 80)
    v128.load
    f64x2.promote_low_f32x4
    f64x2.extract_lane 1
  )

  ;; Verification function to confirm upper lanes are ignored
  ;; Tests that lanes 2 and 3 of input f32x4 don't affect the promotion result
  (func (export "test_promote_ignore_upper") (param $f32_0 f32) (param $f32_1 f32) (result v128)
    ;; Store only lower two lanes, fill upper lanes with distinctive values
    (i32.const 96)
    local.get $f32_0    ;; Lane 0 (promoted)
    f32.store

    (i32.const 100)
    local.get $f32_1    ;; Lane 1 (promoted)
    f32.store

    (i32.const 104)
    (f32.const 99999.0) ;; Lane 2 (should be ignored)
    f32.store

    (i32.const 108)
    (f32.const -88888.0) ;; Lane 3 (should be ignored)
    f32.store

    ;; Load and promote - result should only depend on lanes 0,1
    (i32.const 96)
    v128.load
    f64x2.promote_low_f32x4
  )
)