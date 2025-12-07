(module
  ;; Import memory with 1 page (64KB) for testing
  (memory (export "memory") 1)

  ;; Initialize memory with test data for various i64.load8_s scenarios
  (data (i32.const 0) "\32")           ;; Address 0: 0x32 (50) - positive value
  (data (i32.const 1) "\ce")           ;; Address 1: 0xCE (-50) - negative value
  (data (i32.const 2) "\00")           ;; Address 2: 0x00 (0) - zero value
  (data (i32.const 3) "\7f")           ;; Address 3: 0x7F (127) - max positive
  (data (i32.const 4) "\80")           ;; Address 4: 0x80 (-128) - min negative
  (data (i32.const 5) "\ff")           ;; Address 5: 0xFF (-1) - negative one
  (data (i32.const 6) "\55")           ;; Address 6: 0x55 (85) - alternating bits positive
  (data (i32.const 7) "\aa")           ;; Address 7: 0xAA (-86) - alternating bits negative
  (data (i32.const 8) "\7e")           ;; Address 8: 0x7E (126) - boundary positive
  (data (i32.const 9) "\81")           ;; Address 9: 0x81 (-127) - boundary negative
  (data (i32.const 16) "\01")          ;; Address 16: 0x01 (1) - middle memory
  (data (i32.const 65535) "\02")       ;; Address 65535: 0x02 (2) - end of memory

  ;; Test function: Load positive 8-bit value (0x32 = 50)
  ;; Expected result: 0x0000000000000032 (sign-extended positive)
  (func (export "test_load_positive") (param i32) (result i64)
    i32.const 0
    i64.load8_s
  )

  ;; Test function: Load negative 8-bit value (0xCE = -50)
  ;; Expected result: 0xFFFFFFFFFFFFFFCE (sign-extended negative)
  (func (export "test_load_negative") (param i32) (result i64)
    i32.const 1
    i64.load8_s
  )

  ;; Test function: Load zero value (0x00 = 0)
  ;; Expected result: 0x0000000000000000 (zero with positive sign extension)
  (func (export "test_load_zero") (param i32) (result i64)
    i32.const 2
    i64.load8_s
  )

  ;; Test function: Load maximum positive 8-bit signed value (0x7F = 127)
  ;; Expected result: 0x000000000000007F (max positive with zero extension)
  (func (export "test_load_max_positive") (param i32) (result i64)
    i32.const 3
    i64.load8_s
  )

  ;; Test function: Load minimum negative 8-bit signed value (0x80 = -128)
  ;; Expected result: 0xFFFFFFFFFFFFFF80 (min negative with sign extension)
  (func (export "test_load_min_negative") (param i32) (result i64)
    i32.const 4
    i64.load8_s
  )

  ;; Test function: Load negative one (0xFF = -1)
  ;; Expected result: 0xFFFFFFFFFFFFFFFF (all bits set)
  (func (export "test_load_negative_one") (param i32) (result i64)
    i32.const 5
    i64.load8_s
  )

  ;; Test function: Load from memory start address (address 0)
  ;; Expected result: 0x0000000000000032 (positive value at start)
  (func (export "test_load_address_0") (param i32) (result i64)
    i32.const 0
    i64.load8_s
  )

  ;; Test function: Load from middle memory address
  ;; Expected result: 0x0000000000000001 (value 1 from middle memory)
  (func (export "test_load_address_middle") (param i32) (result i64)
    i32.const 16
    i64.load8_s
  )

  ;; Test function: Load from near end of memory (last valid address)
  ;; Expected result: 0x0000000000000002 (value 2 from end of memory)
  (func (export "test_load_address_end") (param i32) (result i64)
    i32.const 65535
    i64.load8_s
  )

  ;; Test function: Attempt load beyond memory bounds (should trap)
  ;; Expected behavior: WebAssembly trap due to out-of-bounds access
  (func (export "test_load_out_of_bounds_1") (param i32) (result i64)
    i32.const 65536  ;; memory_size (1 page = 65536 bytes)
    i64.load8_s      ;; This should trap - accessing beyond allocated memory
  )

  ;; Test function: Attempt load far beyond memory bounds (should trap)
  ;; Expected behavior: WebAssembly trap due to out-of-bounds access
  (func (export "test_load_out_of_bounds_2") (param i32) (result i64)
    i32.const 65636  ;; memory_size + 100
    i64.load8_s      ;; This should trap - far beyond allocated memory
  )

  ;; Test function: Attempt load at maximum address value (should trap)
  ;; Expected behavior: WebAssembly trap due to out-of-bounds access
  (func (export "test_load_max_address") (param i32) (result i64)
    i32.const 0xFFFFFFFF  ;; Maximum 32-bit address value
    i64.load8_s           ;; This should trap - way beyond any reasonable memory
  )

  ;; Test function: Load alternating bit pattern - positive (0x55 = 0101 0101 = 85)
  ;; Expected result: 0x0000000000000055 (positive alternating pattern)
  (func (export "test_load_pattern_0x55") (param i32) (result i64)
    i32.const 6
    i64.load8_s
  )

  ;; Test function: Load alternating bit pattern - negative (0xAA = 1010 1010 = -86)
  ;; Expected result: 0xFFFFFFFFFFFFFFAA (negative alternating pattern)
  (func (export "test_load_pattern_0xAA") (param i32) (result i64)
    i32.const 7
    i64.load8_s
  )

  ;; Test function: Load boundary positive value (0x7E = 126)
  ;; Expected result: 0x000000000000007E (positive boundary pattern)
  (func (export "test_load_pattern_0x7E") (param i32) (result i64)
    i32.const 8
    i64.load8_s
  )

  ;; Test function: Load boundary negative value (0x81 = -127)
  ;; Expected result: 0xFFFFFFFFFFFFFF81 (negative boundary pattern)
  (func (export "test_load_pattern_0x81") (param i32) (result i64)
    i32.const 9
    i64.load8_s
  )

  ;; Additional validation function: Test with dynamic address parameter
  ;; This function accepts the address as a parameter to test dynamic addressing
  (func (export "test_load_dynamic") (param i32) (result i64)
    local.get 0
    i64.load8_s
  )

  ;; Stack underflow test function: This function tests i64.load8_s with proper stack setup
  ;; but should not cause underflow since we provide the address parameter correctly
  (func (export "test_stack_behavior") (param i32) (result i64)
    ;; Push address onto stack
    i32.const 0
    ;; Perform i64.load8_s operation
    i64.load8_s
    ;; Result should be 0x0000000000000032 (value at address 0)
  )
)