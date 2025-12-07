;; f32x4_pmax_test.wat - WebAssembly Text Format for f32x4.pmax opcode testing
;; Tests comprehensive f32x4.pmax pseudo-maximum operation functionality

(module
  ;; Import memory for test module requirements
  (memory (export "memory") 1)

  ;; Function: test_f32x4_pmax_basic
  ;; Description: Performs f32x4.pmax operation on two input f32x4 vectors
  ;; Parameters: Eight f32 parameters (2 f32x4 vectors flattened)
  ;; Returns: Four f32 results (f32x4 result vector flattened)
  (func (export "test_f32x4_pmax_basic")
    (param $a0 f32) (param $a1 f32) (param $a2 f32) (param $a3 f32)  ;; First f32x4 vector
    (param $b0 f32) (param $b1 f32) (param $b2 f32) (param $b3 f32)  ;; Second f32x4 vector
    (result f32) (result f32) (result f32) (result f32)              ;; Result f32x4 vector
    (local $vec_result v128)

    ;; Build first vector using splat and replace_lane operations
    (f32x4.splat (local.get $a0))
    (f32x4.replace_lane 1 (local.get $a1))
    (f32x4.replace_lane 2 (local.get $a2))
    (f32x4.replace_lane 3 (local.get $a3))

    ;; Build second vector using splat and replace_lane operations
    (f32x4.splat (local.get $b0))
    (f32x4.replace_lane 1 (local.get $b1))
    (f32x4.replace_lane 2 (local.get $b2))
    (f32x4.replace_lane 3 (local.get $b3))

    ;; Perform f32x4.pmax pseudo-maximum operation
    f32x4.pmax
    local.set $vec_result

    ;; Extract individual f32 results from the result vector
    ;; Extract lane 0 result
    (f32x4.extract_lane 0 (local.get $vec_result))
    ;; Extract lane 1 result
    (f32x4.extract_lane 1 (local.get $vec_result))
    ;; Extract lane 2 result
    (f32x4.extract_lane 2 (local.get $vec_result))
    ;; Extract lane 3 result
    (f32x4.extract_lane 3 (local.get $vec_result))
  )

  ;; Function: test_f32x4_pmax_boundary
  ;; Description: Tests f32x4.pmax with boundary values (infinity, FLT_MAX, FLT_MIN, subnormals)
  ;; Parameters: Eight f32 parameters for boundary value testing
  ;; Returns: Four f32 results
  (func (export "test_f32x4_pmax_boundary")
    (param $a0 f32) (param $a1 f32) (param $a2 f32) (param $a3 f32)
    (param $b0 f32) (param $b1 f32) (param $b2 f32) (param $b3 f32)
    (result f32) (result f32) (result f32) (result f32)
    (local $vec_result v128)

    ;; Build vectors for boundary value testing
    (f32x4.splat (local.get $a0))
    (f32x4.replace_lane 1 (local.get $a1))
    (f32x4.replace_lane 2 (local.get $a2))
    (f32x4.replace_lane 3 (local.get $a3))

    (f32x4.splat (local.get $b0))
    (f32x4.replace_lane 1 (local.get $b1))
    (f32x4.replace_lane 2 (local.get $b2))
    (f32x4.replace_lane 3 (local.get $b3))

    ;; Execute f32x4.pmax with boundary values
    f32x4.pmax
    local.set $vec_result

    ;; Extract boundary test results
    (f32x4.extract_lane 0 (local.get $vec_result))
    (f32x4.extract_lane 1 (local.get $vec_result))
    (f32x4.extract_lane 2 (local.get $vec_result))
    (f32x4.extract_lane 3 (local.get $vec_result))
  )

  ;; Function: test_f32x4_pmax_special
  ;; Description: Tests f32x4.pmax with special values (NaN, infinity, signed zeros)
  ;; Parameters: Eight f32 parameters for special value combinations
  ;; Returns: Four f32 results
  (func (export "test_f32x4_pmax_special")
    (param $a0 f32) (param $a1 f32) (param $a2 f32) (param $a3 f32)
    (param $b0 f32) (param $b1 f32) (param $b2 f32) (param $b3 f32)
    (result f32) (result f32) (result f32) (result f32)
    (local $vec_result v128)

    ;; Build vectors for special value testing
    (f32x4.splat (local.get $a0))
    (f32x4.replace_lane 1 (local.get $a1))
    (f32x4.replace_lane 2 (local.get $a2))
    (f32x4.replace_lane 3 (local.get $a3))

    (f32x4.splat (local.get $b0))
    (f32x4.replace_lane 1 (local.get $b1))
    (f32x4.replace_lane 2 (local.get $b2))
    (f32x4.replace_lane 3 (local.get $b3))

    ;; Execute f32x4.pmax with special values (NaN, infinity, signed zero)
    f32x4.pmax
    local.set $vec_result

    ;; Extract special value test results
    (f32x4.extract_lane 0 (local.get $vec_result))
    (f32x4.extract_lane 1 (local.get $vec_result))
    (f32x4.extract_lane 2 (local.get $vec_result))
    (f32x4.extract_lane 3 (local.get $vec_result))
  )

  ;; Function: test_f32x4_pmax_nan_semantics
  ;; Description: Tests f32x4.pmax pseudo-maximum NaN semantics (differs from regular max)
  ;; Parameters: Eight f32 parameters including various NaN patterns
  ;; Returns: Four f32 results validating pmax-specific NaN handling
  (func (export "test_f32x4_pmax_nan_semantics")
    (param $a0 f32) (param $a1 f32) (param $a2 f32) (param $a3 f32)
    (param $b0 f32) (param $b1 f32) (param $b2 f32) (param $b3 f32)
    (result f32) (result f32) (result f32) (result f32)
    (local $vec_result v128)

    ;; Build vectors for NaN semantic testing
    (f32x4.splat (local.get $a0))
    (f32x4.replace_lane 1 (local.get $a1))
    (f32x4.replace_lane 2 (local.get $a2))
    (f32x4.replace_lane 3 (local.get $a3))

    (f32x4.splat (local.get $b0))
    (f32x4.replace_lane 1 (local.get $b1))
    (f32x4.replace_lane 2 (local.get $b2))
    (f32x4.replace_lane 3 (local.get $b3))

    ;; Execute f32x4.pmax to test pseudo-maximum NaN semantics
    ;; When first operand is NaN, pmax should return second operand
    f32x4.pmax
    local.set $vec_result

    ;; Extract NaN semantics test results
    (f32x4.extract_lane 0 (local.get $vec_result))
    (f32x4.extract_lane 1 (local.get $vec_result))
    (f32x4.extract_lane 2 (local.get $vec_result))
    (f32x4.extract_lane 3 (local.get $vec_result))
  )

  ;; Function: test_f32x4_pmax_subnormal
  ;; Description: Tests f32x4.pmax with subnormal (denormalized) float values
  ;; Parameters: Eight f32 parameters including subnormal test values
  ;; Returns: Four f32 results for subnormal value validation
  (func (export "test_f32x4_pmax_subnormal")
    (param $a0 f32) (param $a1 f32) (param $a2 f32) (param $a3 f32)
    (param $b0 f32) (param $b1 f32) (param $b2 f32) (param $b3 f32)
    (result f32) (result f32) (result f32) (result f32)
    (local $vec_result v128)

    ;; Build vectors for subnormal value testing
    (f32x4.splat (local.get $a0))
    (f32x4.replace_lane 1 (local.get $a1))
    (f32x4.replace_lane 2 (local.get $a2))
    (f32x4.replace_lane 3 (local.get $a3))

    (f32x4.splat (local.get $b0))
    (f32x4.replace_lane 1 (local.get $b1))
    (f32x4.replace_lane 2 (local.get $b2))
    (f32x4.replace_lane 3 (local.get $b3))

    ;; Execute f32x4.pmax with subnormal values
    f32x4.pmax
    local.set $vec_result

    ;; Extract subnormal test results
    (f32x4.extract_lane 0 (local.get $vec_result))
    (f32x4.extract_lane 1 (local.get $vec_result))
    (f32x4.extract_lane 2 (local.get $vec_result))
    (f32x4.extract_lane 3 (local.get $vec_result))
  )

  ;; Function: test_f32x4_pmax_zero_sign
  ;; Description: Tests f32x4.pmax with signed zero values (+0.0f vs -0.0f)
  ;; Parameters: Eight f32 parameters for signed zero testing
  ;; Returns: Four f32 results for signed zero validation
  (func (export "test_f32x4_pmax_zero_sign")
    (param $a0 f32) (param $a1 f32) (param $a2 f32) (param $a3 f32)
    (param $b0 f32) (param $b1 f32) (param $b2 f32) (param $b3 f32)
    (result f32) (result f32) (result f32) (result f32)
    (local $vec_result v128)

    ;; Build vectors for signed zero testing
    (f32x4.splat (local.get $a0))
    (f32x4.replace_lane 1 (local.get $a1))
    (f32x4.replace_lane 2 (local.get $a2))
    (f32x4.replace_lane 3 (local.get $a3))

    (f32x4.splat (local.get $b0))
    (f32x4.replace_lane 1 (local.get $b1))
    (f32x4.replace_lane 2 (local.get $b2))
    (f32x4.replace_lane 3 (local.get $b3))

    ;; Execute f32x4.pmax with signed zero combinations
    f32x4.pmax
    local.set $vec_result

    ;; Extract signed zero test results
    (f32x4.extract_lane 0 (local.get $vec_result))
    (f32x4.extract_lane 1 (local.get $vec_result))
    (f32x4.extract_lane 2 (local.get $vec_result))
    (f32x4.extract_lane 3 (local.get $vec_result))
  )

  ;; Function: test_f32x4_pmax_mixed_lanes
  ;; Description: Tests f32x4.pmax with different value types across lanes
  ;; Parameters: Eight f32 parameters for mixed lane testing
  ;; Returns: Four f32 results for cross-lane validation
  (func (export "test_f32x4_pmax_mixed_lanes")
    (param $a0 f32) (param $a1 f32) (param $a2 f32) (param $a3 f32)
    (param $b0 f32) (param $b1 f32) (param $b2 f32) (param $b3 f32)
    (result f32) (result f32) (result f32) (result f32)
    (local $vec_result v128)

    ;; Build vectors for mixed lane testing
    (f32x4.splat (local.get $a0))
    (f32x4.replace_lane 1 (local.get $a1))
    (f32x4.replace_lane 2 (local.get $a2))
    (f32x4.replace_lane 3 (local.get $a3))

    (f32x4.splat (local.get $b0))
    (f32x4.replace_lane 1 (local.get $b1))
    (f32x4.replace_lane 2 (local.get $b2))
    (f32x4.replace_lane 3 (local.get $b3))

    ;; Execute f32x4.pmax with mixed values across all lanes
    f32x4.pmax
    local.set $vec_result

    ;; Extract mixed lane test results
    (f32x4.extract_lane 0 (local.get $vec_result))
    (f32x4.extract_lane 1 (local.get $vec_result))
    (f32x4.extract_lane 2 (local.get $vec_result))
    (f32x4.extract_lane 3 (local.get $vec_result))
  )
)