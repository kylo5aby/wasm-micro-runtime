;; i8x16_extract_lane_s_test.wat - WebAssembly Text Format for i8x16.extract_lane_s opcode testing

(module
  ;; Function: test_extract_positive_lane0
  ;; Description: Extracts a positive value (42) from lane 0 using i8x16.extract_lane_s
  ;; Returns: i32 (sign-extended result from 8-bit value)
  (func $test_extract_positive_lane0 (export "test_extract_positive_lane0")
    (result i32)
    (local $vector v128)

    ;; Create v128 with value 42 in lane 0 and various other values
    (v128.const i8x16 42 -50 0 127 -128 100 -1 64 -64 25 -25 80 -80 10 -10 -25)
    (local.set $vector)

    ;; Extract lane 0 using i8x16.extract_lane_s (signed extraction)
    (local.get $vector)
    (i8x16.extract_lane_s 0)
  )

  ;; Function: test_extract_negative_lane1
  ;; Description: Extracts a negative value (-50) from lane 1 using i8x16.extract_lane_s
  ;; Returns: i32 (sign-extended result from 8-bit value)
  (func $test_extract_negative_lane1 (export "test_extract_negative_lane1")
    (result i32)
    (local $vector v128)

    ;; Create v128 with value -50 in lane 1
    (v128.const i8x16 42 -50 0 127 -128 100 -1 64 -64 25 -25 80 -80 10 -10 -25)
    (local.set $vector)

    ;; Extract lane 1 using i8x16.extract_lane_s (signed extraction)
    (local.get $vector)
    (i8x16.extract_lane_s 1)
  )

  ;; Function: test_extract_zero_lane2
  ;; Description: Extracts zero value from lane 2 using i8x16.extract_lane_s
  ;; Returns: i32 (should be 0)
  (func $test_extract_zero_lane2 (export "test_extract_zero_lane2")
    (result i32)
    (local $vector v128)

    ;; Create v128 with value 0 in lane 2
    (v128.const i8x16 42 -50 0 127 -128 100 -1 64 -64 25 -25 80 -80 10 -10 -25)
    (local.set $vector)

    ;; Extract lane 2 using i8x16.extract_lane_s (signed extraction)
    (local.get $vector)
    (i8x16.extract_lane_s 2)
  )

  ;; Function: test_extract_middle_lane7
  ;; Description: Extracts value from middle lane (7) using i8x16.extract_lane_s
  ;; Returns: i32 (should be 64)
  (func $test_extract_middle_lane7 (export "test_extract_middle_lane7")
    (result i32)
    (local $vector v128)

    ;; Create v128 with value 64 in lane 7
    (v128.const i8x16 42 -50 0 127 -128 100 -1 64 -64 25 -25 80 -80 10 -10 -25)
    (local.set $vector)

    ;; Extract lane 7 using i8x16.extract_lane_s (signed extraction)
    (local.get $vector)
    (i8x16.extract_lane_s 7)
  )

  ;; Function: test_extract_last_lane15
  ;; Description: Extracts value from last lane (15) using i8x16.extract_lane_s
  ;; Returns: i32 (should be -25)
  (func $test_extract_last_lane15 (export "test_extract_last_lane15")
    (result i32)
    (local $vector v128)

    ;; Create v128 with value -25 in lane 15
    (v128.const i8x16 42 -50 0 127 -128 100 -1 64 -64 25 -25 80 -80 10 -10 -25)
    (local.set $vector)

    ;; Extract lane 15 using i8x16.extract_lane_s (signed extraction)
    (local.get $vector)
    (i8x16.extract_lane_s 15)
  )

  ;; Function: test_extract_max_positive
  ;; Description: Extracts maximum positive 8-bit value (127) from lane 0
  ;; Returns: i32 (should be 127)
  (func $test_extract_max_positive (export "test_extract_max_positive")
    (result i32)
    (local $vector v128)

    ;; Create v128 with maximum positive value (127) in lane 0
    (v128.const i8x16 127 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0)
    (local.set $vector)

    ;; Extract lane 0 using i8x16.extract_lane_s (signed extraction)
    (local.get $vector)
    (i8x16.extract_lane_s 0)
  )

  ;; Function: test_extract_max_negative
  ;; Description: Extracts maximum negative 8-bit value (-128) from lane 15
  ;; Returns: i32 (should be -128)
  (func $test_extract_max_negative (export "test_extract_max_negative")
    (result i32)
    (local $vector v128)

    ;; Create v128 with maximum negative value (-128) in lane 15
    (v128.const i8x16 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -128)
    (local.set $vector)

    ;; Extract lane 15 using i8x16.extract_lane_s (signed extraction)
    (local.get $vector)
    (i8x16.extract_lane_s 15)
  )

  ;; Function: test_boundary_first_lane
  ;; Description: Tests boundary value extraction from first lane (lane 0)
  ;; Returns: i32 (should be 127)
  (func $test_boundary_first_lane (export "test_boundary_first_lane")
    (result i32)
    (local $vector v128)

    ;; Create v128 with boundary values
    (v128.const i8x16 127 -128 127 -128 127 -128 127 -128 127 -128 127 -128 127 -128 127 -128)
    (local.set $vector)

    ;; Extract lane 0 using i8x16.extract_lane_s (signed extraction)
    (local.get $vector)
    (i8x16.extract_lane_s 0)
  )

  ;; Function: test_boundary_last_lane
  ;; Description: Tests boundary value extraction from last lane (lane 15)
  ;; Returns: i32 (should be -128)
  (func $test_boundary_last_lane (export "test_boundary_last_lane")
    (result i32)
    (local $vector v128)

    ;; Create v128 with boundary values
    (v128.const i8x16 127 -128 127 -128 127 -128 127 -128 127 -128 127 -128 127 -128 127 -128)
    (local.set $vector)

    ;; Extract lane 15 using i8x16.extract_lane_s (signed extraction)
    (local.get $vector)
    (i8x16.extract_lane_s 15)
  )

  ;; Function: test_extract_negative_one
  ;; Description: Tests sign extension for -1 (0xFF)
  ;; Returns: i32 (should be -1, properly sign-extended)
  (func $test_extract_negative_one (export "test_extract_negative_one")
    (result i32)
    (local $vector v128)

    ;; Create v128 with -1 (0xFF) in lane 0
    (v128.const i8x16 -1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0)
    (local.set $vector)

    ;; Extract lane 0 using i8x16.extract_lane_s (signed extraction)
    (local.get $vector)
    (i8x16.extract_lane_s 0)
  )

  ;; Function: test_extract_negative_64
  ;; Description: Tests sign extension for -64 (0xC0)
  ;; Returns: i32 (should be -64, properly sign-extended)
  (func $test_extract_negative_64 (export "test_extract_negative_64")
    (result i32)
    (local $vector v128)

    ;; Create v128 with -64 (0xC0) in lane 0
    (v128.const i8x16 -64 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0)
    (local.set $vector)

    ;; Extract lane 0 using i8x16.extract_lane_s (signed extraction)
    (local.get $vector)
    (i8x16.extract_lane_s 0)
  )

  ;; Function: test_sign_boundary
  ;; Description: Tests critical sign boundary extraction
  ;; Returns: i32 (should be -1)
  (func $test_sign_boundary (export "test_sign_boundary")
    (result i32)
    (local $vector v128)

    ;; Create v128 with sign boundary pattern
    (v128.const i8x16 -1 1 -1 1 -1 1 -1 1 -1 1 -1 1 -1 1 -1 1)
    (local.set $vector)

    ;; Extract lane 0 using i8x16.extract_lane_s (signed extraction)
    (local.get $vector)
    (i8x16.extract_lane_s 0)
  )

  ;; Function: test_identical_values_lane0
  ;; Description: Tests extraction from lane 0 when all lanes have identical values
  ;; Returns: i32 (should be -42)
  (func $test_identical_values_lane0 (export "test_identical_values_lane0")
    (result i32)
    (local $vector v128)

    ;; Create v128 with identical value (-42) in all lanes
    (v128.const i8x16 -42 -42 -42 -42 -42 -42 -42 -42 -42 -42 -42 -42 -42 -42 -42 -42)
    (local.set $vector)

    ;; Extract lane 0 using i8x16.extract_lane_s (signed extraction)
    (local.get $vector)
    (i8x16.extract_lane_s 0)
  )

  ;; Function: test_identical_values_lane7
  ;; Description: Tests extraction from lane 7 when all lanes have identical values
  ;; Returns: i32 (should be -42)
  (func $test_identical_values_lane7 (export "test_identical_values_lane7")
    (result i32)
    (local $vector v128)

    ;; Create v128 with identical value (-42) in all lanes
    (v128.const i8x16 -42 -42 -42 -42 -42 -42 -42 -42 -42 -42 -42 -42 -42 -42 -42 -42)
    (local.set $vector)

    ;; Extract lane 7 using i8x16.extract_lane_s (signed extraction)
    (local.get $vector)
    (i8x16.extract_lane_s 7)
  )

  ;; Function: test_identical_values_lane15
  ;; Description: Tests extraction from lane 15 when all lanes have identical values
  ;; Returns: i32 (should be -42)
  (func $test_identical_values_lane15 (export "test_identical_values_lane15")
    (result i32)
    (local $vector v128)

    ;; Create v128 with identical value (-42) in all lanes
    (v128.const i8x16 -42 -42 -42 -42 -42 -42 -42 -42 -42 -42 -42 -42 -42 -42 -42 -42)
    (local.set $vector)

    ;; Extract lane 15 using i8x16.extract_lane_s (signed extraction)
    (local.get $vector)
    (i8x16.extract_lane_s 15)
  )
)