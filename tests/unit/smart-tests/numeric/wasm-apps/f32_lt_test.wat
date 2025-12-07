;; f32.lt comprehensive test module
;; Tests IEEE 754 floating-point less-than comparison operation
;; Validates basic comparison, special values, boundary conditions, and NaN handling

(module
  ;; Basic f32.lt comparison function
  ;; Tests fundamental less-than operation with standard floating-point values
  (func (export "test_f32_lt") (param $a f32) (param $b f32) (result i32)
    local.get $a
    local.get $b
    f32.lt
  )

  ;; Special values comparison function
  ;; Tests IEEE 754 special values: +/-0, +/-infinity
  (func (export "test_f32_lt_special") (param $a f32) (param $b f32) (result i32)
    local.get $a
    local.get $b
    f32.lt
  )

  ;; Boundary conditions testing function
  ;; Tests extreme floating-point values and subnormal numbers
  (func (export "test_f32_lt_boundary") (param $a f32) (param $b f32) (result i32)
    local.get $a
    local.get $b
    f32.lt
  )

  ;; NaN handling testing function
  ;; Tests IEEE 754 NaN propagation rules in comparisons
  (func (export "test_f32_lt_nan") (param $a f32) (param $b f32) (result i32)
    local.get $a
    local.get $b
    f32.lt
  )

  ;; Comprehensive test cases combining multiple scenarios
  (func (export "test_f32_lt_comprehensive") (result i32)
    (local $result i32)

    ;; Test 1.0 < 2.0 (should return 1)
    f32.const 1.0
    f32.const 2.0
    f32.lt
    local.set $result

    ;; Verify result is 1 (true)
    local.get $result
    i32.const 1
    i32.eq
    if (result i32)
      ;; Test 3.5 < 1.2 (should return 0)
      f32.const 3.5
      f32.const 1.2
      f32.lt
      i32.eqz ;; Should be 0, so eqz makes it 1 for success
    else
      i32.const 0 ;; Return failure
    end
  )

  ;; Zero comparison test cases
  (func (export "test_f32_lt_zeros") (result i32)
    (local $test1 i32)
    (local $test2 i32)

    ;; Test +0.0 < -0.0 (should return 0, IEEE 754: +0 == -0)
    f32.const 0x0.0p+0    ;; +0.0
    f32.const -0x0.0p+0   ;; -0.0
    f32.lt
    i32.eqz               ;; Should be 0, so eqz makes it 1
    local.set $test1

    ;; Test -0.0 < +0.0 (should return 0, IEEE 754: -0 == +0)
    f32.const -0x0.0p+0   ;; -0.0
    f32.const 0x0.0p+0    ;; +0.0
    f32.lt
    i32.eqz               ;; Should be 0, so eqz makes it 1
    local.set $test2

    ;; Return success only if both tests pass
    local.get $test1
    local.get $test2
    i32.and
  )

  ;; Infinity comparison test cases
  (func (export "test_f32_lt_infinity") (result i32)
    (local $test1 i32)
    (local $test2 i32)
    (local $test3 i32)

    ;; Test 1.0 < +inf (should return 1)
    f32.const 1.0
    f32.const inf
    f32.lt
    local.set $test1

    ;; Test -inf < 1.0 (should return 1)
    f32.const -inf
    f32.const 1.0
    f32.lt
    local.set $test2

    ;; Test +inf < 1.0 (should return 0)
    f32.const inf
    f32.const 1.0
    f32.lt
    i32.eqz ;; Should be 0, so eqz makes it 1
    local.set $test3

    ;; Return success only if all tests pass
    local.get $test1
    local.get $test2
    i32.and
    local.get $test3
    i32.and
  )

  ;; NaN comparison comprehensive test
  (func (export "test_f32_lt_nan_comprehensive") (result i32)
    (local $test1 i32)
    (local $test2 i32)
    (local $test3 i32)

    ;; Test NaN < 1.0 (should return 0)
    f32.const nan
    f32.const 1.0
    f32.lt
    i32.eqz ;; Should be 0, so eqz makes it 1
    local.set $test1

    ;; Test 1.0 < NaN (should return 0)
    f32.const 1.0
    f32.const nan
    f32.lt
    i32.eqz ;; Should be 0, so eqz makes it 1
    local.set $test2

    ;; Test NaN < NaN (should return 0)
    f32.const nan
    f32.const nan
    f32.lt
    i32.eqz ;; Should be 0, so eqz makes it 1
    local.set $test3

    ;; Return success only if all NaN tests pass
    local.get $test1
    local.get $test2
    i32.and
    local.get $test3
    i32.and
  )

  ;; Negative number comparison tests
  (func (export "test_f32_lt_negative") (result i32)
    (local $test1 i32)
    (local $test2 i32)

    ;; Test -5.5 < 2.3 (should return 1)
    f32.const -5.5
    f32.const 2.3
    f32.lt
    local.set $test1

    ;; Test -1.0 < -2.0 (should return 0)
    f32.const -1.0
    f32.const -2.0
    f32.lt
    i32.eqz ;; Should be 0, so eqz makes it 1
    local.set $test2

    ;; Return success only if both tests pass
    local.get $test1
    local.get $test2
    i32.and
  )
)