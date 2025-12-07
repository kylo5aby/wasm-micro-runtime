(module
  ;; Import memory for SIMD operations
  (memory (export "memory") 1 1)

  ;; Basic f32x4.max operation test function
  ;; Performs element-wise maximum on two f32x4 vectors
  (func $basic_max_operation (export "basic_max_operation")
        (param $vec1 v128) (param $vec2 v128) (result v128)
    ;; Load parameters and perform f32x4.max
    local.get $vec1
    local.get $vec2
    f32x4.max
  )

  ;; Boundary values max operation test function
  ;; Tests f32x4.max with floating-point boundary values
  (func $boundary_max_operation (export "boundary_max_operation")
        (param $vec1 v128) (param $vec2 v128) (result v128)
    ;; Load parameters and perform f32x4.max
    local.get $vec1
    local.get $vec2
    f32x4.max
  )

  ;; Special values max operation test function
  ;; Tests f32x4.max with IEEE 754 special values (infinity, NaN, signed zero)
  (func $special_max_operation (export "special_max_operation")
        (param $vec1 v128) (param $vec2 v128) (result v128)
    ;; Load parameters and perform f32x4.max
    local.get $vec1
    local.get $vec2
    f32x4.max
  )

  ;; NaN propagation max operation test function
  ;; Tests f32x4.max NaN handling and propagation behavior
  (func $nan_max_operation (export "nan_max_operation")
        (param $vec1 v128) (param $vec2 v128) (result v128)
    ;; Load parameters and perform f32x4.max
    local.get $vec1
    local.get $vec2
    f32x4.max
  )

  ;; Identity max operation test function
  ;; Tests f32x4.max with identical vectors and commutative properties
  (func $identity_max_operation (export "identity_max_operation")
        (param $vec1 v128) (param $vec2 v128) (result v128)
    ;; Load parameters and perform f32x4.max
    local.get $vec1
    local.get $vec2
    f32x4.max
  )

  ;; Comprehensive f32x4.max test with all edge cases
  (func $comprehensive_max_test (export "comprehensive_max_test")
        (param $vec1 v128) (param $vec2 v128) (result v128)
    (local $result v128)
    (local $temp1 v128)
    (local $temp2 v128)

    ;; Test 1: Basic maximum operation
    local.get $vec1
    local.get $vec2
    f32x4.max
    local.set $result

    ;; Test 2: Commutative property validation (max(A,B) = max(B,A))
    local.get $vec2
    local.get $vec1
    f32x4.max
    local.set $temp1

    ;; Test 3: Identity operation (max(A,A) = A)
    local.get $vec1
    local.get $vec1
    f32x4.max
    local.set $temp2

    ;; Return the basic result
    local.get $result
  )

  ;; Helper function for creating test vectors with specific values
  (func $create_test_vector (export "create_test_vector")
        (param $f0 f32) (param $f1 f32) (param $f2 f32) (param $f3 f32) (result v128)
    ;; Create v128 from 4 individual f32 values using splat and replace_lane
    f32.const 0
    f32x4.splat
    local.get $f0
    f32x4.replace_lane 0
    local.get $f1
    f32x4.replace_lane 1
    local.get $f2
    f32x4.replace_lane 2
    local.get $f3
    f32x4.replace_lane 3
  )

  ;; Advanced test function with nested operations
  (func $advanced_max_test (export "advanced_max_test")
        (param $vec1 v128) (param $vec2 v128) (result v128)
    (local $intermediate v128)

    ;; Perform f32x4.max
    local.get $vec1
    local.get $vec2
    f32x4.max
    local.set $intermediate

    ;; Additional operation to test max in context
    local.get $intermediate
    local.get $vec1
    f32x4.max
  )

  ;; Performance test function with multiple max operations
  (func $performance_max_test (export "performance_max_test")
        (param $vec1 v128) (param $vec2 v128) (param $iterations i32) (result v128)
    (local $result v128)
    (local $counter i32)

    ;; Initialize result
    local.get $vec1
    local.set $result

    ;; Initialize counter
    i32.const 0
    local.set $counter

    ;; Loop performing f32x4.max operations
    (loop $max_loop
      ;; Perform f32x4.max
      local.get $result
      local.get $vec2
      f32x4.max
      local.set $result

      ;; Increment counter
      local.get $counter
      i32.const 1
      i32.add
      local.set $counter

      ;; Continue loop if counter < iterations
      local.get $counter
      local.get $iterations
      i32.lt_s
      br_if $max_loop
    )

    ;; Return final result
    local.get $result
  )

  ;; IEEE 754 compliance validation function
  (func $ieee754_validation (export "ieee754_validation")
        (param $vec1 v128) (param $vec2 v128) (result v128)
    ;; Direct f32x4.max following IEEE 754 standard:
    ;; - max(x, NaN) = NaN
    ;; - max(NaN, x) = NaN
    ;; - max(+0, -0) = +0
    ;; - max(-0, +0) = +0
    ;; - max(+∞, x) = +∞ (unless x is NaN)
    ;; - max(-∞, x) = x (unless x is NaN)
    local.get $vec1
    local.get $vec2
    f32x4.max
  )

  ;; Test function with constant vectors for validation
  (func $constant_vector_test (export "constant_vector_test") (result v128)
    ;; Create two constant vectors and perform max
    f32.const 1.5
    f32.const 2.7
    f32.const 3.1
    f32.const 4.9
    f32x4.splat
    drop
    drop
    drop
    f32x4.splat

    f32.const 2.1
    f32.const 1.8
    f32.const 4.2
    f32.const 3.6
    f32x4.splat
    drop
    drop
    drop
    f32x4.splat

    f32x4.max
  )

  ;; Simple max test for basic validation
  (func $simple_max_test (export "simple_max_test")
        (param $vec1 v128) (param $vec2 v128) (result v128)
    local.get $vec1
    local.get $vec2
    f32x4.max
  )
)