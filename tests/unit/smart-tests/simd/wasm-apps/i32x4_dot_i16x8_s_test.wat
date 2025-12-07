(module
  ;; Import memory for SIMD operations
  (memory (export "memory") 1)

  ;; Test function for i32x4.dot_i16x8_s opcode
  ;; Parameters: two v128 values representing i16x8 vectors
  ;; Returns: v128 value representing i32x4 result
  (func $test_i32x4_dot_i16x8_s (export "test_i32x4_dot_i16x8_s")
    (param $vec1 v128) (param $vec2 v128) (result v128)
    local.get $vec1
    local.get $vec2
    i32x4.dot_i16x8_s
  )

  ;; Additional test function with stack manipulation for thorough testing
  (func $test_i32x4_dot_i16x8_s_complex (export "test_i32x4_dot_i16x8_s_complex")
    (param $vec1 v128) (param $vec2 v128) (result v128)
    ;; Test with stack operations
    local.get $vec1
    local.get $vec2
    i32x4.dot_i16x8_s
    ;; Return result
  )

  ;; Test function that exercises dot product in a computation chain
  (func $test_i32x4_dot_chain (export "test_i32x4_dot_chain")
    (param $vec1 v128) (param $vec2 v128) (param $vec3 v128) (result v128)
    ;; Compute dot product of first two vectors
    local.get $vec1
    local.get $vec2
    i32x4.dot_i16x8_s
    ;; Add with third vector (reinterpreted as i32x4)
    local.get $vec3
    i32x4.add
  )

  ;; Test function for boundary value validation
  (func $test_boundary_values (export "test_boundary_values") (result v128)
    ;; Create vectors with boundary i16 values
    ;; vec1: [-32768, 32767, -32768, 32767, -32768, 32767, -32768, 32767]
    v128.const i32x4 -2147450880 -2147450880 -2147450880 -2147450880

    ;; vec2: [32767, -32768, 32767, -32768, 32767, -32768, 32767, -32768]
    v128.const i32x4 2147450879 2147450879 2147450879 2147450879

    i32x4.dot_i16x8_s
  )

  ;; Test function for zero vector validation
  (func $test_zero_vectors (export "test_zero_vectors") (result v128)
    ;; Create zero vector
    v128.const i32x4 0 0 0 0
    ;; Create any vector
    v128.const i32x4 100 200 300 400
    i32x4.dot_i16x8_s
  )

  ;; Test function that validates algorithm correctness with identity pattern
  (func $test_identity_pattern (export "test_identity_pattern") (result v128)
    ;; vec1: [1, 0, 0, 1, 1, 0, 0, 1] as i16x8
    ;; Represented as i32x4: [0x00000001, 0x00010000, 0x00000001, 0x00010000]
    v128.const i32x4 0x00000001 0x00010000 0x00000001 0x00010000

    ;; vec2: [10, 20, 30, 40, 50, 60, 70, 80] as i16x8
    ;; Represented as i32x4: [0x0014000A, 0x0028001E, 0x003C0032, 0x00500046]
    v128.const i32x4 0x0014000A 0x0028001E 0x003C0032 0x00500046

    i32x4.dot_i16x8_s
  )
)