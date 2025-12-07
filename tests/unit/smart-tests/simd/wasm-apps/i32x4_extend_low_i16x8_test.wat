(module
  ;; Test functions for i32x4.extend_low_i16x8 opcode
  ;; Each function tests different aspects of the sign extension operation

  ;; Import memory if needed (optional, can use local memory)
  (memory 1)

  ;; Function: test_basic_extension
  ;; Tests basic functionality with typical signed values
  ;; Returns: 4 i32 values (the extended low 4 i16 values)
  (func $test_basic_extension (export "test_basic_extension")
    (result i32 i32 i32 i32)

    (local $vec v128)

    ;; Create i16x8 vector with input values: [100, -200, 300, -400, 500, -600, 700, -800]
    (v128.const i16x8 100 -200 300 -400 500 -600 700 -800)
    local.set $vec

    ;; Apply i32x4.extend_low_i16x8_s operation
    local.get $vec
    i32x4.extend_low_i16x8_s

    ;; Extract the 4 i32 results and duplicate for extraction
    local.tee $vec
    i32x4.extract_lane 0

    local.get $vec
    i32x4.extract_lane 1

    local.get $vec
    i32x4.extract_lane 2

    local.get $vec
    i32x4.extract_lane 3
  )

  ;; Function: test_boundary_values
  ;; Tests extreme boundary values including INT16_MIN and INT16_MAX
  (func $test_boundary_values (export "test_boundary_values")
    (result i32 i32 i32 i32)

    (local $vec v128)

    ;; Create i16x8 vector with boundary values: [INT16_MIN, INT16_MAX, 0, -1, 1, 2, 3, 4]
    (v128.const i16x8 -32768 32767 0 -1 1 2 3 4)
    local.set $vec

    ;; Apply i32x4.extend_low_i16x8_s operation
    local.get $vec
    i32x4.extend_low_i16x8_s

    ;; Extract the 4 i32 results
    local.tee $vec
    i32x4.extract_lane 0  ;; INT16_MIN -> -32768

    local.get $vec
    i32x4.extract_lane 1  ;; INT16_MAX -> 32767

    local.get $vec
    i32x4.extract_lane 2  ;; 0 -> 0

    local.get $vec
    i32x4.extract_lane 3  ;; -1 -> -1
  )

  ;; Function: test_zero_values
  ;; Tests zero values in all lanes
  (func $test_zero_values (export "test_zero_values")
    (result i32 i32 i32 i32)

    (local $vec v128)

    ;; Create i16x8 vector with all zeros
    (v128.const i16x8 0 0 0 0 0 0 0 0)
    local.set $vec

    ;; Apply i32x4.extend_low_i16x8_s operation
    local.get $vec
    i32x4.extend_low_i16x8_s

    ;; Extract the 4 i32 results (all should be 0)
    local.tee $vec
    i32x4.extract_lane 0

    local.get $vec
    i32x4.extract_lane 1

    local.get $vec
    i32x4.extract_lane 2

    local.get $vec
    i32x4.extract_lane 3
  )

  ;; Function: test_sign_extension
  ;; Tests sign extension with various negative values
  (func $test_sign_extension (export "test_sign_extension")
    (result i32 i32 i32 i32)

    (local $vec v128)

    ;; Create i16x8 vector with negative values: [-1, -32768, -100, -50, 10, 20, 30, 40]
    (v128.const i16x8 -1 -32768 -100 -50 10 20 30 40)
    local.set $vec

    ;; Apply i32x4.extend_low_i16x8_s operation
    local.get $vec
    i32x4.extend_low_i16x8_s

    ;; Extract the 4 i32 results
    local.tee $vec
    i32x4.extract_lane 0  ;; -1 -> -1

    local.get $vec
    i32x4.extract_lane 1  ;; -32768 -> -32768

    local.get $vec
    i32x4.extract_lane 2  ;; -100 -> -100

    local.get $vec
    i32x4.extract_lane 3  ;; -50 -> -50
  )
)