(module
  ;; Memory definition for v128.load8x8_s testing
  ;; Initial size: 1 page (64KB), Maximum size: 2 pages (128KB)
  (memory (export "memory") 1 2)

  ;; Test function: v128.load8x8_s with memory offset parameter
  ;; Loads 8 bytes from memory starting at given offset, sign-extends to i16x8 vector
  ;; @param offset i32 - memory address offset for load operation
  ;; @return v128 - vector containing 8 sign-extended i16 lanes
  (func (export "test_load8x8_s") (param $offset i32) (result v128)
    ;; Execute v128.load8x8_s instruction
    ;; - Takes i32 offset from parameter
    ;; - Loads 8 consecutive bytes from memory[offset:offset+7]
    ;; - Sign-extends each byte to i16 (preserving sign bit)
    ;; - Returns v128 with 8 i16 lanes
    local.get $offset
    v128.load8x8_s
  )

  ;; Utility function: Set single byte in memory
  ;; Allows C++ tests to populate memory with specific byte patterns
  ;; @param addr i32 - memory address to write byte
  ;; @param value i32 - byte value to write (0-255)
  ;; @return i32 - always returns 0 (success indicator)
  (func (export "set_memory_bytes") (param $addr i32) (param $value i32) (result i32)
    ;; Store byte value at specified memory address
    local.get $addr
    local.get $value
    i32.store8

    ;; Return success (0)
    i32.const 0
  )

  ;; Test function: Basic functionality test with pre-loaded pattern
  ;; Demonstrates v128.load8x8_s with known byte sequence
  ;; @return v128 - result of loading bytes [0x01, 0x7F, 0x80, 0xFF, 0x00, 0x55, 0xAA, 0x33]
  (func (export "test_basic_pattern") (result v128)
    ;; Initialize test pattern in memory
    ;; Pattern: [0x01, 0x7F, 0x80, 0xFF, 0x00, 0x55, 0xAA, 0x33]
    ;; Expected sign extension:
    ;;   0x01 -> 0x0001 (1)
    ;;   0x7F -> 0x007F (127)
    ;;   0x80 -> 0xFF80 (-128)
    ;;   0xFF -> 0xFFFF (-1)
    ;;   0x00 -> 0x0000 (0)
    ;;   0x55 -> 0x0055 (85)
    ;;   0xAA -> 0xFFAA (-86)
    ;;   0x33 -> 0x0033 (51)

    ;; Store test bytes at memory offset 64
    i32.const 64
    i32.const 0x01
    i32.store8

    i32.const 65
    i32.const 0x7F
    i32.store8

    i32.const 66
    i32.const 0x80
    i32.store8

    i32.const 67
    i32.const 0xFF
    i32.store8

    i32.const 68
    i32.const 0x00
    i32.store8

    i32.const 69
    i32.const 0x55
    i32.store8

    i32.const 70
    i32.const 0xAA
    i32.store8

    i32.const 71
    i32.const 0x33
    i32.store8

    ;; Load and sign-extend the 8 bytes
    i32.const 64
    v128.load8x8_s
  )

  ;; Test function: Sign extension boundary validation
  ;; Tests critical values around sign bit boundary (0x7F/0x80)
  ;; @return v128 - result demonstrating sign extension behavior
  (func (export "test_sign_boundary") (result v128)
    ;; Test pattern: [0x7E, 0x7F, 0x80, 0x81, 0x00, 0x01, 0xFE, 0xFF]
    ;; Expected results: [126, 127, -128, -127, 0, 1, -2, -1]

    ;; Store boundary test bytes at memory offset 128
    i32.const 128  ;; 0x7E (126)
    i32.const 0x7E
    i32.store8

    i32.const 129  ;; 0x7F (127) - maximum positive signed byte
    i32.const 0x7F
    i32.store8

    i32.const 130  ;; 0x80 (128) - minimum negative signed byte (-128)
    i32.const 0x80
    i32.store8

    i32.const 131  ;; 0x81 (129) - negative value (-127)
    i32.const 0x81
    i32.store8

    i32.const 132  ;; 0x00 (0) - zero
    i32.const 0x00
    i32.store8

    i32.const 133  ;; 0x01 (1) - positive
    i32.const 0x01
    i32.store8

    i32.const 134  ;; 0xFE (254) - negative value (-2)
    i32.const 0xFE
    i32.store8

    i32.const 135  ;; 0xFF (255) - maximum negative value (-1)
    i32.const 0xFF
    i32.store8

    ;; Execute load with sign extension
    i32.const 128
    v128.load8x8_s
  )

  ;; Test function: All zero pattern load
  ;; Verifies handling of zero bytes
  ;; @return v128 - should contain all zero lanes
  (func (export "test_zero_pattern") (result v128)
    ;; Clear 8 bytes at memory offset 192
    i32.const 192
    i64.const 0
    i64.store

    ;; Load zeros with sign extension (should remain zeros)
    i32.const 192
    v128.load8x8_s
  )

  ;; Test function: All 0xFF pattern load
  ;; Verifies handling of maximum negative bytes
  ;; @return v128 - should contain all -1 lanes (0xFFFF)
  (func (export "test_max_negative_pattern") (result v128)
    ;; Set 8 bytes to 0xFF at memory offset 224
    i32.const 224
    i64.const 0xFFFFFFFFFFFFFFFF
    i64.store

    ;; Load 0xFF bytes with sign extension (should become -1 in each lane)
    i32.const 224
    v128.load8x8_s
  )

  ;; Test function: Alternating pattern test
  ;; Tests mixed positive/negative pattern
  ;; @return v128 - result of loading alternating 0x00/0xFF pattern
  (func (export "test_alternating_pattern") (result v128)
    ;; Pattern: [0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF]
    ;; Expected: [0, -1, 0, -1, 0, -1, 0, -1]

    ;; Store alternating pattern at memory offset 256
    i32.const 256
    i32.const 0x00
    i32.store8

    i32.const 257
    i32.const 0xFF
    i32.store8

    i32.const 258
    i32.const 0x00
    i32.store8

    i32.const 259
    i32.const 0xFF
    i32.store8

    i32.const 260
    i32.const 0x00
    i32.store8

    i32.const 261
    i32.const 0xFF
    i32.store8

    i32.const 262
    i32.const 0x00
    i32.store8

    i32.const 263
    i32.const 0xFF
    i32.store8

    ;; Load alternating pattern
    i32.const 256
    v128.load8x8_s
  )
)