(module
  ;; Memory with 1 page (64KB) for i64.load32_s testing
  (memory (export "memory") 1)

  ;; Data section to initialize memory with test patterns for i64.load32_s
  ;; All 32-bit values stored in little-endian format

  ;; Address 0: 0x12345678 (positive 32-bit value)
  (data (i32.const 0) "\78\56\34\12")

  ;; Address 4: 0x00000001 (small positive value)
  (data (i32.const 4) "\01\00\00\00")

  ;; Address 8: 0x7FFFFFFF (maximum positive signed 32-bit value)
  (data (i32.const 8) "\ff\ff\ff\7f")

  ;; Address 12: 0x87654321 (negative 32-bit value - MSB set)
  (data (i32.const 12) "\21\43\65\87")

  ;; Address 16: 0x80000000 (minimum negative signed 32-bit value)
  (data (i32.const 16) "\00\00\00\80")

  ;; Address 20: 0xFFFFFFFF (all-ones pattern - negative)
  (data (i32.const 20) "\ff\ff\ff\ff")

  ;; Address 24: 0x7FFFFFFF (boundary positive value)
  (data (i32.const 24) "\ff\ff\ff\7f")

  ;; Address 28: 0x80000000 (boundary negative value)
  (data (i32.const 28) "\00\00\00\80")

  ;; Address 32: 0x00000000 (zero pattern)
  (data (i32.const 32) "\00\00\00\00")

  ;; Address 36: 0xAAAAAAAA (alternating pattern - MSB=1, negative)
  (data (i32.const 36) "\aa\aa\aa\aa")

  ;; Address 40: 0x55555555 (alternating pattern - MSB=0, positive)
  (data (i32.const 40) "\55\55\55\55")

  ;; Address 44: 0x12345678 (unaligned test data starting here)
  (data (i32.const 44) "\78\56\34\12\87\65\43\21\ff\ff\00\00")

  ;; Initialize memory boundary area (near 64KB limit)
  ;; Address 65532: Last valid 4-byte boundary for i64.load32_s (65536 - 4)
  (data (i32.const 65532) "\ef\be\ad\de")

  ;; Basic i64.load32_s function - loads signed 32-bit value and sign-extends to 64-bit
  ;; Parameters: address (i32)
  ;; Returns: sign-extended value (i64)
  (func (export "test_i64_load32_s") (param $addr i32) (result i64)
    (i64.load32_s (local.get $addr))
  )

  ;; i64.load32_s with offset=4 - loads from address + 4
  ;; Parameters: base address (i32)
  ;; Returns: sign-extended value from address + 4 (i64)
  (func (export "test_i64_load32_s_offset4") (param $addr i32) (result i64)
    (i64.load32_s offset=4 (local.get $addr))
  )

  ;; i64.load32_s with offset=8 - loads from address + 8
  ;; Parameters: base address (i32)
  ;; Returns: sign-extended value from address + 8 (i64)
  (func (export "test_i64_load32_s_offset8") (param $addr i32) (result i64)
    (i64.load32_s offset=8 (local.get $addr))
  )

  ;; i64.load32_s with offset=16 - loads from address + 16
  ;; Parameters: base address (i32)
  ;; Returns: sign-extended value from address + 16 (i64)
  (func (export "test_i64_load32_s_offset16") (param $addr i32) (result i64)
    (i64.load32_s offset=16 (local.get $addr))
  )

  ;; i64.load32_s with 1-byte alignment hint (align=0)
  ;; Parameters: address (i32)
  ;; Returns: sign-extended value (i64)
  (func (export "test_i64_load32_s_align1") (param $addr i32) (result i64)
    (i64.load32_s align=1 (local.get $addr))
  )

  ;; i64.load32_s with 2-byte alignment hint (align=1)
  ;; Parameters: address (i32)
  ;; Returns: sign-extended value (i64)
  (func (export "test_i64_load32_s_align2") (param $addr i32) (result i64)
    (i64.load32_s align=2 (local.get $addr))
  )

  ;; i64.load32_s with 4-byte alignment hint (align=2)
  ;; Parameters: address (i32)
  ;; Returns: sign-extended value (i64)
  (func (export "test_i64_load32_s_align4") (param $addr i32) (result i64)
    (i64.load32_s align=4 (local.get $addr))
  )

  ;; Sign extension verification function
  ;; Loads positive and negative values to demonstrate sign extension behavior
  ;; Parameters: address (i32)
  ;; Returns: XOR of positive and negative loaded values (i64)
  (func (export "test_sign_extension") (param $addr i32) (result i64)
    (i64.xor
      (i64.load32_s (local.get $addr))           ;; Load positive value (should zero-extend)
      (i64.load32_s offset=12 (local.get $addr)) ;; Load negative value (should sign-extend)
    )
  )

  ;; Boundary value test function - loads from specific boundary addresses
  ;; Tests loading 32-bit values at sign extension boundaries
  ;; Parameters: address (i32)
  ;; Returns: sign-extended value (i64)
  (func (export "test_boundary_load") (param $addr i32) (result i64)
    (i64.load32_s (local.get $addr))
  )

  ;; Stack manipulation test function for i64.load32_s
  ;; Creates a scenario where stack operations with sign-extended i64 can be tested
  ;; Parameters: address (i32)
  ;; Returns: result after stack manipulation (i64)
  (func (export "test_stack_operations") (param $addr i32) (result i64)
    (local $temp i64)
    (local.set $temp (i64.load32_s (local.get $addr)))
    (local.get $temp)
  )

  ;; Endianness validation function for 32-bit values with sign extension
  ;; Loads a multi-byte 32-bit value to verify little-endian interpretation and sign extension
  ;; Parameters: address (i32)
  ;; Returns: sign-extended value for endianness check (i64)
  (func (export "test_endianness") (param $addr i32) (result i64)
    (i64.load32_s (local.get $addr))
  )

  ;; Multi-load test function - performs multiple consecutive i64.load32_s operations
  ;; Tests memory access patterns and sign extension consistency
  ;; Parameters: base address (i32)
  ;; Returns: XOR of multiple sign-extended values (i64)
  (func (export "test_multi_load") (param $addr i32) (result i64)
    (i64.xor
      (i64.xor
        (i64.load32_s (local.get $addr))
        (i64.load32_s offset=4 (local.get $addr))
      )
      (i64.load32_s offset=8 (local.get $addr))
    )
  )

  ;; Error condition test helper - attempts load at potentially invalid address
  ;; Used by C++ tests to verify trap generation for i64.load32_s
  ;; Parameters: address (i32)
  ;; Returns: sign-extended value or traps (i64)
  (func (export "test_error_conditions") (param $addr i32) (result i64)
    (i64.load32_s (local.get $addr))
  )

  ;; Memory initialization helper function for 32-bit values
  ;; Stores a specific 32-bit pattern at given address for dynamic test setup
  ;; Parameters: address (i32), value (i32)
  ;; Returns: address for verification (i32)
  (func (export "store_test_value") (param $addr i32) (param $value i32) (result i32)
    (i32.store (local.get $addr) (local.get $value))
    (local.get $addr)
  )

  ;; Load-store round-trip test function for 32-bit values with sign extension
  ;; Stores a 32-bit value and immediately loads it back with sign extension
  ;; Parameters: address (i32), value (i32)
  ;; Returns: sign-extended value after store (i64)
  (func (export "test_load_store_roundtrip") (param $addr i32) (param $value i32) (result i64)
    (i32.store (local.get $addr) (local.get $value))
    (i64.load32_s (local.get $addr))
  )

  ;; Memory pattern verification function for 32-bit patterns with sign extension
  ;; Loads and verifies specific bit patterns in memory with proper sign extension
  ;; Parameters: address (i32)
  ;; Returns: sign-extended pattern for verification (i64)
  (func (export "test_bit_patterns") (param $addr i32) (result i64)
    (i64.load32_s (local.get $addr))
  )

  ;; Unaligned access test function for i64.load32_s
  ;; Tests i64.load32_s from unaligned memory addresses with sign extension
  ;; Parameters: address (i32)
  ;; Returns: sign-extended value from unaligned address (i64)
  (func (export "test_unaligned_load") (param $addr i32) (result i64)
    (i64.load32_s (local.get $addr))
  )

  ;; Large offset calculation test for 32-bit loads
  ;; Tests complex address calculations with large offsets for i64.load32_s
  ;; Parameters: base address (i32)
  ;; Returns: sign-extended value from calculated address (i64)
  (func (export "test_large_offset") (param $addr i32) (result i64)
    (i64.load32_s offset=1024 (local.get $addr))
  )

  ;; Cross-page boundary test function for 32-bit loads
  ;; Tests i64.load32_s operations that may span virtual memory page boundaries
  ;; Parameters: address (i32)
  ;; Returns: sign-extended value crossing page boundaries (i64)
  (func (export "test_cross_page_load") (param $addr i32) (result i64)
    (i64.load32_s (local.get $addr))
  )

  ;; Maximum positive signed 32-bit value test
  ;; Loads and returns the maximum positive signed 32-bit value with zero extension
  ;; Parameters: none
  ;; Returns: 0x000000007FFFFFFF (i64)
  (func (export "test_max_positive_32bit") (result i64)
    (i64.load32_s (i32.const 8))
  )

  ;; Minimum negative signed 32-bit value test
  ;; Loads and returns the minimum negative signed 32-bit value with sign extension
  ;; Parameters: none
  ;; Returns: 0xFFFFFFFF80000000 (i64)
  (func (export "test_min_negative_32bit") (result i64)
    (i64.load32_s (i32.const 16))
  )

  ;; Zero value test with sign extension
  ;; Loads and returns zero from memory with sign extension (should remain zero)
  ;; Parameters: none
  ;; Returns: 0x0000000000000000 (i64)
  (func (export "test_zero_value") (result i64)
    (i64.load32_s (i32.const 32))
  )

  ;; All-ones 32-bit pattern test
  ;; Loads and returns the all-ones 32-bit pattern with sign extension
  ;; Parameters: none
  ;; Returns: 0xFFFFFFFFFFFFFFFF (i64)
  (func (export "test_all_ones_32bit") (result i64)
    (i64.load32_s (i32.const 20))
  )

  ;; Positive alternating pattern test (0x55555555)
  ;; Loads positive alternating bit pattern and verifies zero extension
  ;; Parameters: none
  ;; Returns: 0x0000000055555555 (i64)
  (func (export "test_positive_alternating") (result i64)
    (i64.load32_s (i32.const 40))
  )

  ;; Negative alternating pattern test (0xAAAAAAAA)
  ;; Loads negative alternating bit pattern and verifies sign extension
  ;; Parameters: none
  ;; Returns: 0xFFFFFFFFAAAAAAAA (i64)
  (func (export "test_negative_alternating") (result i64)
    (i64.load32_s (i32.const 36))
  )

  ;; Sign extension comparison function
  ;; Compares results of loading same 32-bit data with different sign interpretation
  ;; Demonstrates difference between signed and unsigned extension
  ;; Parameters: address (i32)
  ;; Returns: signed extended value (i64)
  (func (export "test_sign_comparison") (param $addr i32) (result i64)
    (i64.load32_s (local.get $addr))
  )

  ;; Memory boundary edge case test
  ;; Tests loading from the last valid 4-byte boundary in memory
  ;; Parameters: none
  ;; Returns: sign-extended value from boundary address (i64)
  (func (export "test_memory_boundary") (result i64)
    (i64.load32_s (i32.const 65532))
  )

  ;; Complex computation with sign extension
  ;; Performs arithmetic operations on sign-extended values
  ;; Parameters: base address (i32)
  ;; Returns: result of computation with sign-extended values (i64)
  (func (export "test_computation_with_sign_ext") (param $addr i32) (result i64)
    (i64.add
      (i64.load32_s (local.get $addr))
      (i64.load32_s offset=4 (local.get $addr))
    )
  )
)