(module
  ;; Memory with 1 page (64KB) for i32.load8_u testing
  (memory (export "memory") 1)

  ;; Data section to initialize memory with test byte patterns
  ;; Layout designed to test zero extension behavior for unsigned loads
  (data (i32.const 0) "\42")                ;; 0x42 at address 0 (should zero-extend to 0x00000042)
  (data (i32.const 1) "\7F")                ;; 0x7F at address 1 (max signed positive, zero-extend to 0x0000007F)
  (data (i32.const 2) "\AB")                ;; 0xAB at address 2 (unsigned pattern, zero-extend to 0x000000AB)
  (data (i32.const 3) "\00")                ;; 0x00 at address 3 (zero, should remain 0x00000000)
  (data (i32.const 4) "\CD")                ;; 0xCD at address 4 (for offset testing, zero-extend to 0x000000CD)

  ;; Boundary value test patterns
  (data (i32.const 8) "\00")                ;; 0x00 at address 8 (minimum unsigned value)
  (data (i32.const 9) "\FF")                ;; 0xFF at address 9 (maximum unsigned value -> 0x000000FF)
  (data (i32.const 10) "\7F")               ;; 0x7F at address 10 (sign boundary positive -> 0x0000007F)
  (data (i32.const 11) "\80")               ;; 0x80 at address 11 (sign boundary negative -> 0x00000080, NOT sign-extended)

  ;; Additional test patterns for comprehensive zero extension validation
  (data (i32.const 12) "\01")               ;; 0x01 at address 12 (minimal non-zero)
  (data (i32.const 13) "\FE")               ;; 0xFE at address 13 (near maximum -> 0x000000FE)
  (data (i32.const 14) "\55")               ;; 0x55 at address 14 (alternating bit pattern)
  (data (i32.const 15) "\AA")               ;; 0xAA at address 15 (alternating bit pattern -> 0x000000AA)

  ;; Memory boundary test data (near 64KB limit)
  (data (i32.const 65535) "\3C")            ;; Test data at last valid byte address

  ;; Basic i32.load8_u function - loads unsigned 8-bit value and zero-extends to 32-bit
  ;; Parameters: address (i32)
  ;; Returns: zero-extended value (i32)
  (func (export "test_i32_load8_u") (param $addr i32) (result i32)
    (i32.load8_u (local.get $addr))
  )

  ;; i32.load8_u with offset=1 - loads from address + 1
  ;; Parameters: base address (i32)
  ;; Returns: zero-extended value from address + 1 (i32)
  (func (export "test_i32_load8_u_offset1") (param $addr i32) (result i32)
    (i32.load8_u offset=1 (local.get $addr))
  )

  ;; i32.load8_u with offset=2 - loads from address + 2
  ;; Parameters: base address (i32)
  ;; Returns: zero-extended value from address + 2 (i32)
  (func (export "test_i32_load8_u_offset2") (param $addr i32) (result i32)
    (i32.load8_u offset=2 (local.get $addr))
  )

  ;; i32.load8_u with offset=4 - loads from address + 4
  ;; Parameters: base address (i32)
  ;; Returns: zero-extended value from address + 4 (i32)
  (func (export "test_i32_load8_u_offset4") (param $addr i32) (result i32)
    (i32.load8_u offset=4 (local.get $addr))
  )

  ;; Comparison function: i32.load8_s for demonstrating extension difference
  ;; This function loads the same byte using signed load for comparison
  ;; Parameters: address (i32)
  ;; Returns: sign-extended value (i32) - for comparison with unsigned variant
  (func (export "test_i32_load8_s_compare") (param $addr i32) (result i32)
    (i32.load8_s (local.get $addr))
  )

  ;; Memory initialization helper for dynamic testing
  ;; Stores a byte value at specified address for runtime test data setup
  ;; Parameters: address (i32), value (i32)
  (func (export "store_test_byte") (param $addr i32) (param $value i32)
    (i32.store8 (local.get $addr) (local.get $value))
  )

  ;; Memory bounds test helper - attempts load from potentially invalid address
  ;; This function will trap if address is out of bounds, used for error testing
  ;; Parameters: address (i32)
  ;; Returns: value (i32) or traps if out of bounds
  (func (export "test_bounds_load") (param $addr i32) (result i32)
    (i32.load8_u (local.get $addr))
  )

  ;; Zero extension verification helper
  ;; Loads byte and masks upper bits to verify they are zero
  ;; Parameters: address (i32)
  ;; Returns: upper 24 bits (should always be 0 for proper zero extension)
  (func (export "verify_zero_extension") (param $addr i32) (result i32)
    (i32.and
      (i32.load8_u (local.get $addr))
      (i32.const 0xFFFFFF00)  ;; Mask to check upper 24 bits
    )
  )

  ;; Pattern comparison helper - loads same address with both signed and unsigned
  ;; Returns XOR of both results to highlight extension differences
  ;; Parameters: address (i32)
  ;; Returns: XOR result highlighting difference between extensions
  (func (export "compare_extensions") (param $addr i32) (result i32)
    (i32.xor
      (i32.load8_u (local.get $addr))  ;; Zero-extended value
      (i32.load8_s (local.get $addr))  ;; Sign-extended value
    )
  )

  ;; Memory size helper function
  ;; Returns current memory size in bytes (pages * 65536)
  ;; Used for bounds testing and validation
  (func (export "get_memory_size") (result i32)
    (i32.mul
      (memory.size)     ;; Returns size in pages
      (i32.const 65536) ;; Convert to bytes (1 page = 64KB)
    )
  )
)