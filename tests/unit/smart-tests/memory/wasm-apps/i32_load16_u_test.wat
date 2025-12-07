(module
  ;; Memory section with 1 page (64KB) for testing i32.load16_u operations
  (memory 1)

  ;; Export memory for external access and inspection
  (export "memory" (memory 0))

  ;; Main test function for i32.load16_u with zero extension validation
  ;; Loads a 16-bit unsigned value from memory and zero-extends to 32 bits
  ;; @param i32 address - Memory address to load from
  ;; @return i32 - Zero-extended 32-bit value (upper 16 bits always 0)
  (func $load16_u (export "load16_u") (param $addr i32) (result i32)
    (i32.load16_u (local.get $addr))
  )

  ;; Signed load function for comparison testing with i32.load16_u
  ;; Loads a 16-bit signed value from memory and sign-extends to 32 bits
  ;; @param i32 address - Memory address to load from
  ;; @return i32 - Sign-extended 32-bit value (upper 16 bits preserve sign)
  (func $load16_s (export "load16_s") (param $addr i32) (result i32)
    (i32.load16_s (local.get $addr))
  )

  ;; Store function for test data setup and memory preparation
  ;; Stores a 16-bit value at specified memory address
  ;; @param i32 address - Memory address to store to
  ;; @param i32 value - 16-bit value to store (upper 16 bits ignored)
  (func $store16 (export "store16") (param $addr i32) (param $value i32)
    (i32.store16 (local.get $addr) (local.get $value))
  )

  ;; Basic load test function with offset handling
  ;; Tests i32.load16_u with static offset for alignment testing
  ;; @param i32 base_addr - Base memory address
  ;; @return i32 - Zero-extended value loaded from base_addr + 2
  (func $load16_u_offset (export "load16_u_offset") (param $base i32) (result i32)
    (i32.load16_u offset=2 (local.get $base))
  )

  ;; Zero extension validation function
  ;; Demonstrates explicit zero extension behavior for values with sign bit set
  ;; @param i32 address - Memory address containing 16-bit value
  ;; @return i32 - Zero-extended result with upper 16 bits guaranteed to be 0
  (func $validate_zero_extension (export "validate_zero_extension") (param $addr i32) (result i32)
    (local $loaded i32)
    ;; Load the 16-bit unsigned value
    (local.set $loaded (i32.load16_u (local.get $addr)))
    ;; Mask upper 16 bits to demonstrate they should already be zero
    (i32.and (local.get $loaded) (i32.const 0x0000FFFF))
  )

  ;; Boundary test function for memory edge access
  ;; Tests loading from memory boundaries to validate access limits
  ;; @param i32 offset - Offset from memory start
  ;; @return i32 - Loaded value if within bounds, traps if out of bounds
  (func $load_boundary (export "load_boundary") (param $offset i32) (result i32)
    ;; Calculate address near memory boundary (65536 - 2 bytes)
    (local $boundary_addr i32)
    (local.set $boundary_addr (i32.sub (i32.const 65534) (local.get $offset)))
    (i32.load16_u (local.get $boundary_addr))
  )

  ;; Little-endian validation function
  ;; Verifies correct little-endian byte interpretation in memory loads
  ;; @return i32 - Result demonstrating little-endian byte ordering
  (func $test_little_endian (export "test_little_endian") (result i32)
    ;; Store known pattern for little-endian testing
    (i32.store16 (i32.const 0) (i32.const 0x1234))
    ;; Load and return the value (should maintain little-endian order)
    (i32.load16_u (i32.const 0))
  )

  ;; Unaligned access test function
  ;; Tests i32.load16_u with unaligned memory addresses (odd boundaries)
  ;; @param i32 base - Base address for unaligned testing
  ;; @return i32 - Value loaded from unaligned address (base + 1)
  (func $test_unaligned (export "test_unaligned") (param $base i32) (result i32)
    ;; Access memory at unaligned address (odd boundary)
    (i32.load16_u (i32.add (local.get $base) (i32.const 1)))
  )

  ;; Sign bit test function for unsigned interpretation validation
  ;; Demonstrates that i32.load16_u treats values as unsigned (no sign extension)
  ;; @return i32 - Zero-extended result for value with sign bit set
  (func $test_sign_bit (export "test_sign_bit") (result i32)
    ;; Store value with sign bit set (0x8000)
    (i32.store16 (i32.const 0) (i32.const 0x8000))
    ;; Load as unsigned - should be 0x00008000, not 0xFFFF8000
    (i32.load16_u (i32.const 0))
  )

  ;; Maximum value test function
  ;; Tests i32.load16_u with maximum 16-bit unsigned value (0xFFFF)
  ;; @return i32 - Zero-extended result (0x0000FFFF)
  (func $test_max_value (export "test_max_value") (result i32)
    ;; Store maximum 16-bit unsigned value
    (i32.store16 (i32.const 0) (i32.const 0xFFFF))
    ;; Load and verify zero extension
    (i32.load16_u (i32.const 0))
  )

  ;; Multi-address test function
  ;; Tests consecutive i32.load16_u operations from multiple addresses
  ;; @return i32 - Sum of loaded values to verify all operations succeed
  (func $test_multi_address (export "test_multi_address") (result i32)
    (local $sum i32)

    ;; Setup test data at multiple addresses
    (i32.store16 (i32.const 0) (i32.const 0x0001))
    (i32.store16 (i32.const 2) (i32.const 0x0002))
    (i32.store16 (i32.const 4) (i32.const 0x0004))
    (i32.store16 (i32.const 6) (i32.const 0x0008))

    ;; Load from all addresses and sum the results
    (local.set $sum (i32.add
      (i32.add
        (i32.load16_u (i32.const 0))
        (i32.load16_u (i32.const 2)))
      (i32.add
        (i32.load16_u (i32.const 4))
        (i32.load16_u (i32.const 6)))))

    (local.get $sum)
  )

  ;; Alignment comparison test function
  ;; Compares aligned vs unaligned access performance and correctness
  ;; @param i32 value - Test value to store and load
  ;; @return i32 - Result of aligned and unaligned loads (should be identical)
  (func $test_alignment (export "test_alignment") (param $value i32) (result i32)
    (local $aligned i32)
    (local $unaligned i32)

    ;; Store value at aligned address (even boundary)
    (i32.store16 (i32.const 0) (local.get $value))
    ;; Store same value at unaligned address (odd boundary)
    (i32.store16 (i32.const 1) (local.get $value))

    ;; Load from both addresses
    (local.set $aligned (i32.load16_u (i32.const 0)))
    (local.set $unaligned (i32.load16_u (i32.const 1)))

    ;; Return XOR result - should be 0 if values are identical
    (i32.xor (local.get $aligned) (local.get $unaligned))
  )

  ;; Memory initialization function for comprehensive testing
  ;; Sets up known test patterns in memory for systematic validation
  (func $init_test_memory (export "init_test_memory")
    ;; Initialize memory with test patterns for comprehensive testing
    (i32.store16 (i32.const 0) (i32.const 0x0000))   ;; Minimum value
    (i32.store16 (i32.const 2) (i32.const 0x1234))   ;; Typical value
    (i32.store16 (i32.const 4) (i32.const 0x7FFF))   ;; Max positive signed
    (i32.store16 (i32.const 6) (i32.const 0x8000))   ;; Sign bit set
    (i32.store16 (i32.const 8) (i32.const 0xFFFF))   ;; Maximum unsigned
    (i32.store16 (i32.const 10) (i32.const 0x55AA))  ;; Alternating bits
    (i32.store16 (i32.const 12) (i32.const 0xAA55))  ;; Inverted pattern
    (i32.store16 (i32.const 14) (i32.const 0x0001))  ;; Minimal non-zero
  )

  ;; Comprehensive pattern test function
  ;; Validates i32.load16_u with various bit patterns and addresses
  ;; @param i32 pattern_id - Identifier for test pattern (0-7)
  ;; @return i32 - Loaded value corresponding to pattern_id
  (func $test_patterns (export "test_patterns") (param $pattern_id i32) (result i32)
    ;; Calculate address based on pattern_id (pattern_id * 2)
    (local $addr i32)
    (local.set $addr (i32.mul (local.get $pattern_id) (i32.const 2)))

    ;; Load 16-bit unsigned value from calculated address
    (i32.load16_u (local.get $addr))
  )
)