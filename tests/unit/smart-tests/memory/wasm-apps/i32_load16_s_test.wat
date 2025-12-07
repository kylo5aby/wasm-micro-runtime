(module
  ;; Memory with 1 page (64KB) for i32.load16_s testing
  (memory (export "memory") 1)

  ;; Data section to initialize memory with test patterns for 16-bit signed load testing
  ;; Each data entry contains 16-bit values in little-endian format

  ;; Positive 16-bit values (should remain positive when sign-extended)
  (data (i32.const 0) "\34\12")             ;; 0x1234 at address 0
  (data (i32.const 2) "\78\56")             ;; 0x5678 at address 2
  (data (i32.const 4) "\ff\7f")             ;; 0x7FFF at address 4 (max positive signed 16-bit)

  ;; Negative 16-bit values (should sign-extend with 0xFFFF prefix)
  (data (i32.const 6) "\00\80")             ;; 0x8000 at address 6 (min negative signed 16-bit)
  (data (i32.const 8) "\ff\ff")             ;; 0xFFFF at address 8 (max unsigned/min signed -1)
  (data (i32.const 10) "\bc\9a")            ;; 0x9ABC at address 10 (negative value)

  ;; Special test values
  (data (i32.const 12) "\00\00")            ;; 0x0000 at address 12 (zero value)
  (data (i32.const 14) "\ef\be")            ;; 0xBEEF at address 14 (negative value)
  (data (i32.const 16) "\ad\de")            ;; 0xDEAD at address 16 (negative value)

  ;; Additional test patterns for unaligned access testing
  (data (i32.const 18) "\12\34\56\78")      ;; Multi-byte pattern for unaligned tests
  (data (i32.const 22) "\9a\bc\de\f0")      ;; Another multi-byte pattern

  ;; Sign extension validation patterns
  (data (i32.const 26) "\fe\7f")            ;; 0x7FFE (positive)
  (data (i32.const 28) "\01\80")            ;; 0x8001 (negative)
  (data (i32.const 30) "\aa\55")            ;; 0x55AA (positive)
  (data (i32.const 32) "\55\aa")            ;; 0xAA55 (negative)

  ;; Boundary test data near memory limits
  (data (i32.const 65534) "\34\12")         ;; Test data at last valid 16-bit address

  ;; Basic i32.load16_s function - loads 16-bit signed value and sign-extends to 32-bit
  ;; Parameters: address (i32)
  ;; Returns: sign-extended 16-bit value (i32)
  (func (export "test_load16_s") (param $addr i32) (result i32)
    (i32.load16_s (local.get $addr))
  )

  ;; i32.load16_s with offset=2 - loads from address + 2
  ;; Parameters: base address (i32)
  ;; Returns: sign-extended 16-bit value from address + 2 (i32)
  (func (export "test_load16_s_offset2") (param $addr i32) (result i32)
    (i32.load16_s offset=2 (local.get $addr))
  )

  ;; i32.load16_s with offset=4 - loads from address + 4
  ;; Parameters: base address (i32)
  ;; Returns: sign-extended 16-bit value from address + 4 (i32)
  (func (export "test_load16_s_offset4") (param $addr i32) (result i32)
    (i32.load16_s offset=4 (local.get $addr))
  )

  ;; i32.load16_s with 1-byte alignment hint (align=0)
  ;; Parameters: address (i32)
  ;; Returns: sign-extended 16-bit value (i32)
  (func (export "test_load16_s_align1") (param $addr i32) (result i32)
    (i32.load16_s align=1 (local.get $addr))
  )

  ;; i32.load16_s with 2-byte alignment hint (align=1)
  ;; Parameters: address (i32)
  ;; Returns: sign-extended 16-bit value (i32)
  (func (export "test_load16_s_align2") (param $addr i32) (result i32)
    (i32.load16_s align=2 (local.get $addr))
  )

  ;; Sign extension validation function - loads positive value
  ;; Tests that positive 16-bit values remain positive with zero-extended upper bits
  ;; Parameters: address (i32)
  ;; Returns: sign-extended positive value (i32)
  (func (export "test_positive_sign_ext") (param $addr i32) (result i32)
    (i32.load16_s (local.get $addr))
  )

  ;; Sign extension validation function - loads negative value
  ;; Tests that negative 16-bit values are properly sign-extended with 0xFFFF upper bits
  ;; Parameters: address (i32)
  ;; Returns: sign-extended negative value (i32)
  (func (export "test_negative_sign_ext") (param $addr i32) (result i32)
    (i32.load16_s (local.get $addr))
  )

  ;; Boundary value test function - loads maximum positive signed 16-bit value
  ;; Tests loading 0x7FFF and verifying it extends to 0x00007FFF
  ;; Returns: 0x00007FFF (i32)
  (func (export "test_max_positive_16bit") (result i32)
    (i32.load16_s (i32.const 4))  ;; Load 0x7FFF from address 4
  )

  ;; Boundary value test function - loads minimum negative signed 16-bit value
  ;; Tests loading 0x8000 and verifying it extends to 0xFFFF8000
  ;; Returns: 0xFFFF8000 (i32)
  (func (export "test_min_negative_16bit") (result i32)
    (i32.load16_s (i32.const 6))  ;; Load 0x8000 from address 6
  )

  ;; Zero value test function
  ;; Tests loading zero value and verifying proper sign extension
  ;; Returns: 0x00000000 (i32)
  (func (export "test_zero_value") (result i32)
    (i32.load16_s (i32.const 12)) ;; Load 0x0000 from address 12
  )

  ;; Maximum unsigned/minimum signed test function
  ;; Tests loading 0xFFFF (-1 in signed) and verifying it extends to 0xFFFFFFFF
  ;; Returns: 0xFFFFFFFF (i32)
  (func (export "test_max_unsigned_16bit") (result i32)
    (i32.load16_s (i32.const 8))  ;; Load 0xFFFF from address 8
  )

  ;; Unaligned access test function
  ;; Tests loading from unaligned addresses (odd addresses)
  ;; Parameters: address (i32)
  ;; Returns: sign-extended 16-bit value from unaligned address (i32)
  (func (export "test_unaligned_load") (param $addr i32) (result i32)
    (i32.load16_s (local.get $addr))
  )

  ;; Little-endian verification function
  ;; Tests that bytes are interpreted in correct little-endian order
  ;; Loads a known pattern and verifies byte order interpretation
  ;; Parameters: address (i32)
  ;; Returns: value demonstrating little-endian byte order (i32)
  (func (export "test_endianness_16bit") (param $addr i32) (result i32)
    (i32.load16_s (local.get $addr))
  )

  ;; Multiple load test function
  ;; Performs multiple i32.load16_s operations to test consistency
  ;; Parameters: base address (i32)
  ;; Returns: XOR of multiple loaded 16-bit values (i32)
  (func (export "test_multi_load16_s") (param $addr i32) (result i32)
    (i32.xor
      (i32.xor
        (i32.load16_s (local.get $addr))
        (i32.load16_s offset=2 (local.get $addr))
      )
      (i32.load16_s offset=4 (local.get $addr))
    )
  )

  ;; Sign bit test function - tests values around sign bit transition
  ;; Loads values just below and above 0x8000 to verify sign bit handling
  ;; Parameters: test_case (i32) - 0 for 0x7FFE, 1 for 0x7FFF, 2 for 0x8000, 3 for 0x8001
  ;; Returns: sign-extended value for specified test case (i32)
  (func (export "test_sign_bit_transition") (param $test_case i32) (result i32)
    (if (result i32) (i32.eq (local.get $test_case) (i32.const 0))
      (then (i32.load16_s (i32.const 26))) ;; Load 0x7FFE
      (else
        (if (result i32) (i32.eq (local.get $test_case) (i32.const 1))
          (then (i32.load16_s (i32.const 4)))  ;; Load 0x7FFF
          (else
            (if (result i32) (i32.eq (local.get $test_case) (i32.const 2))
              (then (i32.load16_s (i32.const 6)))  ;; Load 0x8000
              (else (i32.load16_s (i32.const 28))) ;; Load 0x8001
            )
          )
        )
      )
    )
  )

  ;; Memory boundary test function
  ;; Tests loading from last valid 16-bit address in memory
  ;; Returns: value from memory boundary address (i32)
  (func (export "test_memory_boundary") (result i32)
    (i32.load16_s (i32.const 65534))  ;; Last valid 16-bit address
  )

  ;; Error condition test helper function
  ;; Used by C++ tests to verify trap generation for out-of-bounds access
  ;; Parameters: address (i32)
  ;; Returns: loaded value or traps (i32)
  (func (export "test_out_of_bounds") (param $addr i32) (result i32)
    (i32.load16_s (local.get $addr))
  )

  ;; Store-load round-trip test for 16-bit values
  ;; Stores a 32-bit value as 16-bit and loads it back with sign extension
  ;; Parameters: address (i32), value (i32)
  ;; Returns: sign-extended loaded value (i32)
  (func (export "test_store_load_roundtrip") (param $addr i32) (param $value i32) (result i32)
    (i32.store16 (local.get $addr) (local.get $value))
    (i32.load16_s (local.get $addr))
  )

  ;; Complex computation test using i32.load16_s
  ;; Demonstrates sign extension in arithmetic context
  ;; Parameters: addr1 (i32), addr2 (i32)
  ;; Returns: sum of two sign-extended 16-bit values (i32)
  (func (export "test_arithmetic_with_load16_s") (param $addr1 i32) (param $addr2 i32) (result i32)
    (i32.add
      (i32.load16_s (local.get $addr1))
      (i32.load16_s (local.get $addr2))
    )
  )

  ;; Sign extension verification with known patterns
  ;; Tests specific bit patterns to verify correct sign extension algorithm
  ;; Parameters: pattern_id (i32) - selects which pattern to test
  ;; Returns: sign-extended value for verification (i32)
  (func (export "test_sign_extension_patterns") (param $pattern_id i32) (result i32)
    (if (result i32) (i32.eq (local.get $pattern_id) (i32.const 0))
      (then (i32.load16_s (i32.const 30))) ;; Load 0x55AA (positive)
      (else (i32.load16_s (i32.const 32))) ;; Load 0xAA55 (negative)
    )
  )

  ;; Advanced offset test with 16-bit loads
  ;; Tests address calculation with offsets for 16-bit operations
  ;; Parameters: base address (i32)
  ;; Returns: sign-extended value from offset address (i32)
  (func (export "test_offset_calculation") (param $addr i32) (result i32)
    (i32.load16_s offset=10 (local.get $addr))
  )
)