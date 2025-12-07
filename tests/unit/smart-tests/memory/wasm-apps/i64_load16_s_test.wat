(module
  ;; Import memory with 1 page (64KB) for testing
  (memory (export "memory") 1)

  ;; Initialize memory with test data for various i64.load16_s scenarios
  ;; Using little-endian byte order (LSB first in memory)
  (data (i32.const 0) "\34\12")           ;; Address 0-1: 0x1234 - positive 16-bit value
  (data (i32.const 2) "\00\80")           ;; Address 2-3: 0x8000 - min negative 16-bit value
  (data (i32.const 4) "\ff\7f")           ;; Address 4-5: 0x7FFF - max positive 16-bit value
  (data (i32.const 6) "\00\00")           ;; Address 6-7: 0x0000 - zero value
  (data (i32.const 8) "\ff\ff")           ;; Address 8-9: 0xFFFF - negative one (-1)

  ;; Test data for aligned access (even addresses)
  (data (i32.const 10) "\cd\ab")          ;; Address 10-11: 0xABCD - negative value (little-endian)
  (data (i32.const 12) "\ef\be")          ;; Address 12-13: 0xBEEF - negative value

  ;; Test data for unaligned access (odd addresses)
  (data (i32.const 13) "\78\56")          ;; Address 13-14: 0x5678 - spans previous data

  ;; Test data near end of memory for boundary testing
  (data (i32.const 65532) "\21\43")       ;; Address 65532-65533: 0x4321 - near end
  (data (i32.const 65534) "\65\87")       ;; Address 65534-65535: 0x8765 - last valid 16-bit

  ;; Test function: Load positive 16-bit value (0x1234 = 4660)
  ;; Expected result: 0x0000000000001234 (sign-extended positive)
  (func (export "test_load_positive") (param i32) (result i64)
    i32.const 0
    i64.load16_s
  )

  ;; Test function: Load minimum negative 16-bit value (0x8000 = -32768)
  ;; Expected result: 0xFFFFFFFFFFFF8000 (sign-extended negative)
  (func (export "test_load_negative") (param i32) (result i64)
    i32.const 2
    i64.load16_s
  )

  ;; Test function: Load maximum positive 16-bit value (0x7FFF = 32767)
  ;; Expected result: 0x0000000000007FFF (sign-extended positive)
  (func (export "test_load_max_positive") (param i32) (result i64)
    i32.const 4
    i64.load16_s
  )

  ;; Test function: Load zero value (0x0000 = 0)
  ;; Expected result: 0x0000000000000000 (zero with positive sign extension)
  (func (export "test_load_zero") (param i32) (result i64)
    i32.const 6
    i64.load16_s
  )

  ;; Test function: Load maximum positive boundary value (0x7FFF = 32767)
  ;; Expected result: 0x0000000000007FFF (boundary positive)
  (func (export "test_load_boundary_max_positive") (param i32) (result i64)
    i32.const 4
    i64.load16_s
  )

  ;; Test function: Load minimum negative boundary value (0x8000 = -32768)
  ;; Expected result: 0xFFFFFFFFFFFF8000 (boundary negative)
  (func (export "test_load_boundary_min_negative") (param i32) (result i64)
    i32.const 2
    i64.load16_s
  )

  ;; Test function: Load negative one (0xFFFF = -1)
  ;; Expected result: 0xFFFFFFFFFFFFFFFF (all bits set)
  (func (export "test_load_boundary_negative_one") (param i32) (result i64)
    i32.const 8
    i64.load16_s
  )

  ;; Test function: Load from memory start address (address 0)
  ;; Expected result: 0x0000000000001234 (positive value at start)
  (func (export "test_load_address_0") (param i32) (result i64)
    i32.const 0
    i64.load16_s
  )

  ;; Test function: Load from aligned address (address 10, even)
  ;; Expected result: 0xFFFFFFFFFFFFABCD (negative value ABCD sign-extended)
  (func (export "test_load_address_aligned") (param i32) (result i64)
    i32.const 10
    i64.load16_s
  )

  ;; Test function: Load from unaligned address (address 13, odd)
  ;; Expected result: 0x0000000000005678 (positive value 5678 sign-extended)
  (func (export "test_load_address_unaligned") (param i32) (result i64)
    i32.const 13
    i64.load16_s
  )

  ;; Test function: Load from valid address near end of memory
  ;; Expected result: 0x0000000000004321 (positive value at memory end)
  (func (export "test_load_address_end") (param i32) (result i64)
    i32.const 65532
    i64.load16_s
  )

  ;; Test function: Attempt load beyond memory bounds (should trap)
  ;; Expected behavior: WebAssembly trap due to out-of-bounds access
  (func (export "test_load_out_of_bounds_1") (param i32) (result i64)
    i32.const 65536  ;; memory_size (1 page = 65536 bytes)
    i64.load16_s     ;; This should trap - accessing beyond allocated memory
  )

  ;; Test function: Attempt load far beyond memory bounds (should trap)
  ;; Expected behavior: WebAssembly trap due to out-of-bounds access
  (func (export "test_load_out_of_bounds_2") (param i32) (result i64)
    i32.const 65636  ;; memory_size + 100
    i64.load16_s     ;; This should trap - far beyond allocated memory
  )

  ;; Test function: Attempt load at boundary requiring 2 bytes but insufficient space (should trap)
  ;; Expected behavior: WebAssembly trap due to insufficient space for 16-bit access
  (func (export "test_load_boundary_insufficient") (param i32) (result i64)
    i32.const 65535  ;; Last byte of memory, but need 2 bytes for 16-bit load
    i64.load16_s     ;; This should trap - insufficient space for 16-bit access
  )

  ;; Test function: Little-endian byte order test
  ;; Memory [0x34, 0x12] → 0x1234 (little-endian interpretation)
  ;; Expected result: 0x0000000000001234 (correct little-endian loading)
  (func (export "test_load_little_endian") (param i32) (result i64)
    i32.const 0  ;; Points to memory containing [0x34, 0x12]
    i64.load16_s
  )

  ;; Test function: Little-endian negative value test
  ;; Memory [0x00, 0x80] → 0x8000 (little-endian interpretation)
  ;; Expected result: 0xFFFFFFFFFFFF8000 (negative little-endian with sign extension)
  (func (export "test_load_negative_little_endian") (param i32) (result i64)
    i32.const 2  ;; Points to memory containing [0x00, 0x80]
    i64.load16_s
  )

  ;; Additional validation function: Test with dynamic address parameter
  ;; This function accepts the address as a parameter to test dynamic addressing
  (func (export "test_load_dynamic") (param i32) (result i64)
    local.get 0
    i64.load16_s
  )

  ;; Stack behavior test function: Tests i64.load16_s with proper stack setup
  ;; Should not cause underflow since we provide the address parameter correctly
  (func (export "test_stack_behavior") (param i32) (result i64)
    ;; Push address onto stack
    i32.const 0
    ;; Perform i64.load16_s operation
    i64.load16_s
    ;; Result should be 0x0000000000001234 (value at address 0-1)
  )

  ;; Comprehensive boundary test: Load from last valid 16-bit position
  ;; Expected result: 0xFFFFFFFFFFFF8765 (negative value 8765 sign-extended)
  (func (export "test_load_last_valid_position") (param i32) (result i64)
    i32.const 65534  ;; Last valid position for 16-bit load
    i64.load16_s
  )

  ;; Test function: Verify sign extension bit pattern preservation
  ;; Load 0xABCD (negative) and verify lower 16 bits preserved, upper 48 bits set
  (func (export "test_sign_extension_verification") (param i32) (result i64)
    i32.const 10     ;; Address containing 0xABCD
    i64.load16_s
  )

  ;; Memory content verification helper: Returns a known value for testing framework
  (func (export "test_memory_content") (param i32) (result i64)
    ;; Return a sentinel value to verify test framework is working
    i64.const 0x123456789ABCDEF0
  )
)