(module
  ;; Memory with 1 page (64KB) for i32.load8_s testing
  (memory (export "memory") 1)

  ;; Data section to initialize memory with test byte patterns
  ;; Layout designed to test sign extension behavior
  (data (i32.const 0) "\42")                ;; 0x42 at address 0 (positive, should extend to 0x00000042)
  (data (i32.const 1) "\00")                ;; 0x00 at address 1 (zero, should extend to 0x00000000)
  (data (i32.const 2) "\7F")                ;; 0x7F at address 2 (max positive, should extend to 0x0000007F)
  (data (i32.const 3) "\80")                ;; 0x80 at address 3 (min negative, should extend to 0xFFFFFF80)
  (data (i32.const 4) "\7E")                ;; 0x7E at address 4 (boundary positive, should extend to 0x0000007E)
  (data (i32.const 5) "\7F")                ;; 0x7F at address 5 (max positive boundary)
  (data (i32.const 6) "\80")                ;; 0x80 at address 6 (min negative boundary)
  (data (i32.const 7) "\81")                ;; 0x81 at address 7 (boundary negative, should extend to 0xFFFFFF81)
  (data (i32.const 8) "\00")                ;; 0x00 at address 8 (extreme: zero)
  (data (i32.const 9) "\FF")                ;; 0xFF at address 9 (extreme: -1, should extend to 0xFFFFFFFF)

  ;; Additional test patterns for comprehensive validation
  (data (i32.const 10) "\01")               ;; 0x01 at address 10 (minimal positive)
  (data (i32.const 11) "\FE")               ;; 0xFE at address 11 (-2, should extend to 0xFFFFFFFE)
  (data (i32.const 12) "\55")               ;; 0x55 at address 12 (positive pattern)
  (data (i32.const 13) "\AA")               ;; 0xAA at address 13 (negative pattern, should extend to 0xFFFFFFAA)
  (data (i32.const 14) "\CC")               ;; 0xCC at address 14 (negative pattern, should extend to 0xFFFFFFCC)
  (data (i32.const 15) "\33")               ;; 0x33 at address 15 (positive pattern)

  ;; Memory boundary test data (near 64KB limit)
  (data (i32.const 65535) "\3C")            ;; Test data at last valid byte address

  ;; Basic i32.load8_s function - loads signed 8-bit value and sign-extends to 32-bit
  ;; Parameters: address (i32)
  ;; Returns: sign-extended value (i32)
  (func (export "test_i32_load8_s") (param $addr i32) (result i32)
    (i32.load8_s (local.get $addr))
  )

  ;; i32.load8_s with offset=1 - loads from address + 1
  ;; Parameters: base address (i32)
  ;; Returns: sign-extended value from address + 1 (i32)
  (func (export "test_i32_load8_s_offset1") (param $addr i32) (result i32)
    (i32.load8_s offset=1 (local.get $addr))
  )

  ;; i32.load8_s with offset=2 - loads from address + 2
  ;; Parameters: base address (i32)
  ;; Returns: sign-extended value from address + 2 (i32)
  (func (export "test_i32_load8_s_offset2") (param $addr i32) (result i32)
    (i32.load8_s offset=2 (local.get $addr))
  )

  ;; i32.load8_s with offset=4 - loads from address + 4
  ;; Parameters: base address (i32)
  ;; Returns: sign-extended value from address + 4 (i32)
  (func (export "test_i32_load8_s_offset4") (param $addr i32) (result i32)
    (i32.load8_s offset=4 (local.get $addr))
  )

  ;; i32.load8_s with 1-byte alignment (align=1, natural alignment for 8-bit access)
  ;; Parameters: address (i32)
  ;; Returns: sign-extended value (i32)
  (func (export "test_i32_load8_s_align1") (param $addr i32) (result i32)
    (i32.load8_s align=1 (local.get $addr))
  )

  ;; Test function for positive byte values (0x00 - 0x7F)
  ;; Parameters: address (i32)
  ;; Returns: sign-extended positive value (i32)
  (func (export "test_positive_bytes") (param $addr i32) (result i32)
    (i32.load8_s (local.get $addr))
  )

  ;; Test function for negative byte values (0x80 - 0xFF)
  ;; Parameters: address (i32)
  ;; Returns: sign-extended negative value (i32)
  (func (export "test_negative_bytes") (param $addr i32) (result i32)
    (i32.load8_s (local.get $addr))
  )

  ;; Combined test function that loads and validates sign extension
  ;; Parameters: address (i32)
  ;; Returns: 1 if sign extension is correct, 0 otherwise (i32)
  (func (export "validate_sign_extension") (param $addr i32) (result i32)
    (local $loaded i32)
    (local $expected i32)

    ;; Load the byte value
    (local.set $loaded (i32.load8_s (local.get $addr)))

    ;; For demonstration: load the same byte as unsigned and check sign extension
    ;; If bit 7 is set (value >= 0x80), result should have all upper bits set
    (local.set $expected
      (i32.load8_u (local.get $addr))) ;; Load as unsigned first

    ;; Check if the loaded signed value matches expected sign extension
    ;; This is a validation helper function for complex sign extension tests
    (if (result i32) (i32.ge_u (local.get $expected) (i32.const 0x80))
      (then
        ;; Negative byte: check if upper bits are set correctly (0xFFFFFF__)
        (i32.eq
          (i32.and (local.get $loaded) (i32.const 0xFFFFFF00))
          (i32.const 0xFFFFFF00)))
      (else
        ;; Positive byte: check if upper bits are clear (0x000000__)
        (i32.eq
          (i32.and (local.get $loaded) (i32.const 0xFFFFFF00))
          (i32.const 0x00000000))))
  )

  ;; Memory bounds test function - should trap on out-of-bounds access
  ;; Parameters: address (i32) - intentionally out of bounds
  ;; Returns: should trap before returning (i32)
  (func (export "test_out_of_bounds") (param $addr i32) (result i32)
    (i32.load8_s (local.get $addr))
  )

  ;; Edge case test: load from maximum valid address
  ;; Parameters: none
  ;; Returns: value from address 65535 (last valid byte) (i32)
  (func (export "test_max_valid_address") (result i32)
    (i32.load8_s (i32.const 65535))
  )

  ;; Performance test function: multiple loads for consistency validation
  ;; Parameters: base address (i32)
  ;; Returns: sum of multiple loads (i32)
  (func (export "test_multiple_loads") (param $base i32) (result i32)
    (i32.add
      (i32.add
        (i32.load8_s (local.get $base))
        (i32.load8_s (i32.add (local.get $base) (i32.const 1))))
      (i32.add
        (i32.load8_s (i32.add (local.get $base) (i32.const 2)))
        (i32.load8_s (i32.add (local.get $base) (i32.const 3)))))
  )
)