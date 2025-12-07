(module
  ;; Import memory if needed
  (memory (export "memory") 1)

  ;; Test function for f32x4.min operation
  ;; Takes 8 f32 parameters (2 f32x4 vectors) and returns 4 f32 values (1 f32x4 result)
  (func (export "test_f32x4_min")
    (param $a0 f32) (param $a1 f32) (param $a2 f32) (param $a3 f32)  ;; First f32x4 vector
    (param $b0 f32) (param $b1 f32) (param $b2 f32) (param $b3 f32)  ;; Second f32x4 vector
    (result f32) (result f32) (result f32) (result f32)              ;; Result f32x4 vector
    (local $vec_result v128)

    ;; Build first vector using splat and replace_lane
    (f32x4.splat (local.get $a0))
    (f32x4.replace_lane 1 (local.get $a1))
    (f32x4.replace_lane 2 (local.get $a2))
    (f32x4.replace_lane 3 (local.get $a3))

    ;; Build second vector using splat and replace_lane
    (f32x4.splat (local.get $b0))
    (f32x4.replace_lane 1 (local.get $b1))
    (f32x4.replace_lane 2 (local.get $b2))
    (f32x4.replace_lane 3 (local.get $b3))

    ;; Perform f32x4.min operation
    f32x4.min
    local.set $vec_result

    ;; Extract individual f32 results from the result vector
    ;; Extract lane 0
    (f32x4.extract_lane 0 (local.get $vec_result))

    ;; Extract lane 1
    (f32x4.extract_lane 1 (local.get $vec_result))

    ;; Extract lane 2
    (f32x4.extract_lane 2 (local.get $vec_result))

    ;; Extract lane 3
    (f32x4.extract_lane 3 (local.get $vec_result))
  )

  ;; Helper function for testing basic minimum with predefined values
  (func (export "test_basic_min") (result f32) (result f32) (result f32) (result f32)
    (local $result v128)
    ;; Test: min([1.5, -2.5, 3.0, -4.0], [2.0, -1.5, 2.5, -5.0])
    ;; Expected result: [1.5, -2.5, 2.5, -5.0]

    ;; First vector: [1.5, -2.5, 3.0, -4.0]
    (f32x4.splat (f32.const 1.5))
    (f32x4.replace_lane 1 (f32.const -2.5))
    (f32x4.replace_lane 2 (f32.const 3.0))
    (f32x4.replace_lane 3 (f32.const -4.0))

    ;; Second vector: [2.0, -1.5, 2.5, -5.0]
    (f32x4.splat (f32.const 2.0))
    (f32x4.replace_lane 1 (f32.const -1.5))
    (f32x4.replace_lane 2 (f32.const 2.5))
    (f32x4.replace_lane 3 (f32.const -5.0))

    ;; Perform f32x4.min
    f32x4.min
    local.set $result

    ;; Extract results
    (f32x4.extract_lane 0 (local.get $result))
    (f32x4.extract_lane 1 (local.get $result))
    (f32x4.extract_lane 2 (local.get $result))
    (f32x4.extract_lane 3 (local.get $result))
  )

  ;; Test function for infinity handling
  (func (export "test_infinity_min") (result f32) (result f32) (result f32) (result f32)
    (local $result v128)
    ;; Test: min([+inf, -inf, +inf, 1.0], [1.0, 1.0, -inf, +inf])
    ;; Expected result: [1.0, -inf, -inf, 1.0]

    ;; First vector: [+inf, -inf, +inf, 1.0]
    (f32x4.splat (f32.const inf))
    (f32x4.replace_lane 1 (f32.const -inf))
    (f32x4.replace_lane 2 (f32.const inf))
    (f32x4.replace_lane 3 (f32.const 1.0))

    ;; Second vector: [1.0, 1.0, -inf, +inf]
    (f32x4.splat (f32.const 1.0))
    (f32x4.replace_lane 1 (f32.const 1.0))
    (f32x4.replace_lane 2 (f32.const -inf))
    (f32x4.replace_lane 3 (f32.const inf))

    ;; Perform f32x4.min
    f32x4.min
    local.set $result

    ;; Extract results
    (f32x4.extract_lane 0 (local.get $result))
    (f32x4.extract_lane 1 (local.get $result))
    (f32x4.extract_lane 2 (local.get $result))
    (f32x4.extract_lane 3 (local.get $result))
  )

  ;; Test function for signed zero handling
  (func (export "test_signed_zero_min") (result f32) (result f32) (result f32) (result f32)
    (local $result v128)
    ;; Test: min([+0.0, -0.0, +0.0, 1.0], [-0.0, +0.0, +0.0, -0.0])
    ;; Expected result: [-0.0, -0.0, +0.0, -0.0]

    ;; First vector: [+0.0, -0.0, +0.0, 1.0]
    (f32x4.splat (f32.const 0.0))
    (f32x4.replace_lane 1 (f32.const -0.0))
    (f32x4.replace_lane 2 (f32.const 0.0))
    (f32x4.replace_lane 3 (f32.const 1.0))

    ;; Second vector: [-0.0, +0.0, +0.0, -0.0]
    (f32x4.splat (f32.const -0.0))
    (f32x4.replace_lane 1 (f32.const 0.0))
    (f32x4.replace_lane 2 (f32.const 0.0))
    (f32x4.replace_lane 3 (f32.const -0.0))

    ;; Perform f32x4.min
    f32x4.min
    local.set $result

    ;; Extract results
    (f32x4.extract_lane 0 (local.get $result))
    (f32x4.extract_lane 1 (local.get $result))
    (f32x4.extract_lane 2 (local.get $result))
    (f32x4.extract_lane 3 (local.get $result))
  )

  ;; Test function for NaN propagation
  (func (export "test_nan_min") (result f32) (result f32) (result f32) (result f32)
    (local $result v128)
    ;; Test: min([NaN, 1.0, NaN, 2.0], [1.0, NaN, NaN, NaN])
    ;; Expected result: [NaN, NaN, NaN, NaN]

    ;; First vector: [NaN, 1.0, NaN, 2.0]
    (f32x4.splat (f32.const nan:0x400000))
    (f32x4.replace_lane 1 (f32.const 1.0))
    (f32x4.replace_lane 2 (f32.const nan:0x400000))
    (f32x4.replace_lane 3 (f32.const 2.0))

    ;; Second vector: [1.0, NaN, NaN, NaN]
    (f32x4.splat (f32.const 1.0))
    (f32x4.replace_lane 1 (f32.const nan:0x400000))
    (f32x4.replace_lane 2 (f32.const nan:0x400000))
    (f32x4.replace_lane 3 (f32.const nan:0x400000))

    ;; Perform f32x4.min
    f32x4.min
    local.set $result

    ;; Extract results
    (f32x4.extract_lane 0 (local.get $result))
    (f32x4.extract_lane 1 (local.get $result))
    (f32x4.extract_lane 2 (local.get $result))
    (f32x4.extract_lane 3 (local.get $result))
  )
)