(module
  ;; Memory definition for v128.load8x8_u testing
  ;; Initial size: 1 page (64KB), Maximum size: 2 pages (128KB)
  (memory (export "memory") 1 2)

  ;; Test function: v128.load8x8_u with memory offset parameter
  ;; Loads 8 bytes from memory starting at given offset, zero-extends to i16x8 vector
  ;; @param offset i32 - memory address offset for load operation
  ;; @return v128 - vector containing 8 zero-extended i16 lanes
  (func (export "test_load8x8_u") (param $offset i32) (result v128)
    ;; Execute v128.load8x8_u instruction
    ;; - Takes i32 offset from parameter
    ;; - Loads 8 consecutive bytes from memory[offset:offset+7]
    ;; - Zero-extends each byte to i16 (always positive values 0x0000-0x00FF)
    ;; - Returns v128 with 8 i16 lanes
    local.get $offset
    v128.load8x8_u
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

  ;; Test function: Out-of-bounds load attempt
  ;; Demonstrates trap behavior when accessing memory beyond allocated bounds
  ;; @param offset i32 - memory offset that should be out-of-bounds
  ;; @return v128 - should never return due to trap
  (func (export "test_out_of_bounds_load") (param $offset i32) (result v128)
    ;; Attempt to load from potentially out-of-bounds offset
    ;; This should trap if offset + 8 > memory.size
    local.get $offset
    v128.load8x8_u
  )

  ;; Test function: Basic functionality test with pre-loaded pattern
  ;; Demonstrates v128.load8x8_u with known byte sequence
  ;; @return v128 - result of loading bytes [0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08]
  (func (export "test_sequential_pattern") (result v128)
    ;; Initialize sequential test pattern in memory
    ;; Pattern: [0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08]
    ;; Expected zero extension:
    ;;   0x01 -> 0x0001 (1)
    ;;   0x02 -> 0x0002 (2)
    ;;   0x03 -> 0x0003 (3)
    ;;   0x04 -> 0x0004 (4)
    ;;   0x05 -> 0x0005 (5)
    ;;   0x06 -> 0x0006 (6)
    ;;   0x07 -> 0x0007 (7)
    ;;   0x08 -> 0x0008 (8)

    ;; Store sequential test bytes at memory offset 64
    i32.const 64
    i32.const 0x01
    i32.store8

    i32.const 65
    i32.const 0x02
    i32.store8

    i32.const 66
    i32.const 0x03
    i32.store8

    i32.const 67
    i32.const 0x04
    i32.store8

    i32.const 68
    i32.const 0x05
    i32.store8

    i32.const 69
    i32.const 0x06
    i32.store8

    i32.const 70
    i32.const 0x07
    i32.store8

    i32.const 71
    i32.const 0x08
    i32.store8

    ;; Load and zero-extend the 8 bytes
    i32.const 64
    v128.load8x8_u
  )

  ;; Test function: Alternating pattern test
  ;; Tests checkerboard byte pattern for zero extension validation
  ;; @return v128 - result demonstrating zero extension behavior
  (func (export "test_alternating_pattern") (result v128)
    ;; Test pattern: [0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA]
    ;; Expected results: [85, 170, 85, 170, 85, 170, 85, 170] (all positive)

    ;; Store alternating test bytes at memory offset 128
    i32.const 128  ;; 0x55 (85)
    i32.const 0x55
    i32.store8

    i32.const 129  ;; 0xAA (170)
    i32.const 0xAA
    i32.store8

    i32.const 130  ;; 0x55 (85)
    i32.const 0x55
    i32.store8

    i32.const 131  ;; 0xAA (170)
    i32.const 0xAA
    i32.store8

    i32.const 132  ;; 0x55 (85)
    i32.const 0x55
    i32.store8

    i32.const 133  ;; 0xAA (170)
    i32.const 0xAA
    i32.store8

    i32.const 134  ;; 0x55 (85)
    i32.const 0x55
    i32.store8

    i32.const 135  ;; 0xAA (170)
    i32.const 0xAA
    i32.store8

    ;; Load and zero-extend the 8 bytes
    i32.const 128
    v128.load8x8_u
  )

  ;; Test function: Maximum byte values test
  ;; Tests zero extension behavior with maximum byte values (0xFF)
  ;; @return v128 - result showing zero-extended maximum values
  (func (export "test_maximum_bytes") (result v128)
    ;; Test pattern: [0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF]
    ;; Expected results: [255, 255, 255, 255, 255, 255, 255, 255] (all 0x00FF)

    ;; Store maximum byte values at memory offset 192
    i32.const 192
    i32.const 0xFF
    i32.store8

    i32.const 193
    i32.const 0xFF
    i32.store8

    i32.const 194
    i32.const 0xFF
    i32.store8

    i32.const 195
    i32.const 0xFF
    i32.store8

    i32.const 196
    i32.const 0xFF
    i32.store8

    i32.const 197
    i32.const 0xFF
    i32.store8

    i32.const 198
    i32.const 0xFF
    i32.store8

    i32.const 199
    i32.const 0xFF
    i32.store8

    ;; Load and zero-extend the 8 maximum bytes
    i32.const 192
    v128.load8x8_u
  )

  ;; Test function: Zero bytes test
  ;; Tests zero extension behavior with all zero bytes
  ;; @return v128 - result showing zero-extended zero values
  (func (export "test_zero_bytes") (result v128)
    ;; Test pattern: [0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00]
    ;; Expected results: [0, 0, 0, 0, 0, 0, 0, 0] (all 0x0000)

    ;; Store zero byte values at memory offset 256
    i32.const 256
    i32.const 0x00
    i32.store8

    i32.const 257
    i32.const 0x00
    i32.store8

    i32.const 258
    i32.const 0x00
    i32.store8

    i32.const 259
    i32.const 0x00
    i32.store8

    i32.const 260
    i32.const 0x00
    i32.store8

    i32.const 261
    i32.const 0x00
    i32.store8

    i32.const 262
    i32.const 0x00
    i32.store8

    i32.const 263
    i32.const 0x00
    i32.store8

    ;; Load and zero-extend the 8 zero bytes
    i32.const 256
    v128.load8x8_u
  )

  ;; Test function: Power-of-two pattern test
  ;; Tests zero extension with power-of-two byte values
  ;; @return v128 - result showing zero-extended power-of-two values
  (func (export "test_power_of_two_pattern") (result v128)
    ;; Test pattern: [0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80]
    ;; Expected results: [1, 2, 4, 8, 16, 32, 64, 128] (all zero-extended)

    ;; Store power-of-two test bytes at memory offset 320
    i32.const 320  ;; 0x01 (1)
    i32.const 0x01
    i32.store8

    i32.const 321  ;; 0x02 (2)
    i32.const 0x02
    i32.store8

    i32.const 322  ;; 0x04 (4)
    i32.const 0x04
    i32.store8

    i32.const 323  ;; 0x08 (8)
    i32.const 0x08
    i32.store8

    i32.const 324  ;; 0x10 (16)
    i32.const 0x10
    i32.store8

    i32.const 325  ;; 0x20 (32)
    i32.const 0x20
    i32.store8

    i32.const 326  ;; 0x40 (64)
    i32.const 0x40
    i32.store8

    i32.const 327  ;; 0x80 (128)
    i32.const 0x80
    i32.store8

    ;; Load and zero-extend the 8 power-of-two bytes
    i32.const 320
    v128.load8x8_u
  )
)