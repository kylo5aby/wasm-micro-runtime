(module
  ;; Import memory with 1 page (64KB) for testing
  (memory (export "memory") 1)

  ;; Initialize memory with test data for various i64.load8_u scenarios
  (data (i32.const 0) "\42")           ;; Address 0: 0x42 (66) - typical value
  (data (i32.const 1) "\7a")           ;; Address 1: 0x7A (122) - medium value
  (data (i32.const 2) "\c3")           ;; Address 2: 0xC3 (195) - high unsigned value
  (data (i32.const 3) "\1f")           ;; Address 3: 0x1F (31) - low value
  (data (i32.const 4) "\00")           ;; Address 4: 0x00 (0) - minimum value
  (data (i32.const 5) "\7f")           ;; Address 5: 0x7F (127) - max positive signed
  (data (i32.const 6) "\80")           ;; Address 6: 0x80 (128) - min negative signed as unsigned
  (data (i32.const 7) "\ff")           ;; Address 7: 0xFF (255) - maximum unsigned value
  (data (i32.const 8) "\55")           ;; Address 8: 0x55 (85) - alternating bits (0101 0101)
  (data (i32.const 9) "\aa")           ;; Address 9: 0xAA (170) - alternating bits (1010 1010)
  (data (i32.const 10) "\0f")          ;; Address 10: 0x0F (15) - nibble pattern (0000 1111)
  (data (i32.const 11) "\f0")          ;; Address 11: 0xF0 (240) - nibble pattern (1111 0000)
  (data (i32.const 16) "\01")          ;; Address 16: 0x01 (1) - middle memory
  (data (i32.const 65535) "\02")       ;; Address 65535: 0x02 (2) - end of memory

  ;; Test function: Load 0x42 (66) - typical unsigned 8-bit value
  ;; Expected result: 0x0000000000000042 (zero-extended)
  (func (export "test_load_0x42") (param i32) (result i64)
    i32.const 0
    i64.load8_u
  )

  ;; Test function: Load 0x7A (122) - medium unsigned value
  ;; Expected result: 0x000000000000007A (zero-extended)
  (func (export "test_load_0x7A") (param i32) (result i64)
    i32.const 1
    i64.load8_u
  )

  ;; Test function: Load 0xC3 (195) - high unsigned value
  ;; Expected result: 0x00000000000000C3 (zero-extended, not sign-extended)
  (func (export "test_load_0xC3") (param i32) (result i64)
    i32.const 2
    i64.load8_u
  )

  ;; Test function: Load 0x1F (31) - low unsigned value
  ;; Expected result: 0x000000000000001F (zero-extended)
  (func (export "test_load_0x1F") (param i32) (result i64)
    i32.const 3
    i64.load8_u
  )

  ;; Test function: Load 0x00 (0) - minimum unsigned value
  ;; Expected result: 0x0000000000000000 (zero remains zero)
  (func (export "test_load_0x00") (param i32) (result i64)
    i32.const 4
    i64.load8_u
  )

  ;; Test function: Load 0x7F (127) - maximum positive signed value as unsigned
  ;; Expected result: 0x000000000000007F (zero-extended, same as signed)
  (func (export "test_load_0x7F") (param i32) (result i64)
    i32.const 5
    i64.load8_u
  )

  ;; Test function: Load 0x80 (128) - minimum negative signed value as unsigned
  ;; Expected result: 0x0000000000000080 (zero-extended, unsigned interpretation)
  (func (export "test_load_0x80") (param i32) (result i64)
    i32.const 6
    i64.load8_u
  )

  ;; Test function: Load 0xFF (255) - maximum unsigned value
  ;; Expected result: 0x00000000000000FF (zero-extended, not 0xFFFFFFFFFFFFFFFF)
  (func (export "test_load_0xFF") (param i32) (result i64)
    i32.const 7
    i64.load8_u
  )

  ;; Test function: Load alternating bit pattern 0x55 (85 = 0101 0101)
  ;; Expected result: 0x0000000000000055 (zero-extended alternating pattern)
  (func (export "test_load_0x55") (param i32) (result i64)
    i32.const 8
    i64.load8_u
  )

  ;; Test function: Load alternating bit pattern 0xAA (170 = 1010 1010)
  ;; Expected result: 0x00000000000000AA (zero-extended, unsigned interpretation)
  (func (export "test_load_0xAA") (param i32) (result i64)
    i32.const 9
    i64.load8_u
  )

  ;; Test function: Load nibble pattern 0x0F (15 = 0000 1111)
  ;; Expected result: 0x000000000000000F (zero-extended nibble)
  (func (export "test_load_0x0F") (param i32) (result i64)
    i32.const 10
    i64.load8_u
  )

  ;; Test function: Load nibble pattern 0xF0 (240 = 1111 0000)
  ;; Expected result: 0x00000000000000F0 (zero-extended high nibble)
  (func (export "test_load_0xF0") (param i32) (result i64)
    i32.const 11
    i64.load8_u
  )

  ;; Test function: Load from memory address 0
  ;; Expected result: 0x0000000000000042 (value at start of memory)
  (func (export "test_load_address_0") (param i32) (result i64)
    i32.const 0
    i64.load8_u
  )

  ;; Test function: Load from middle memory address
  ;; Expected result: 0x0000000000000001 (value 1 from middle memory)
  (func (export "test_load_address_middle") (param i32) (result i64)
    i32.const 16
    i64.load8_u
  )

  ;; Test function: Load from near end of memory (last valid address)
  ;; Expected result: 0x0000000000000002 (value 2 from end of memory)
  (func (export "test_load_address_end") (param i32) (result i64)
    i32.const 65535
    i64.load8_u
  )

  ;; Test function: Attempt load beyond memory bounds (should trap)
  ;; Expected behavior: WebAssembly trap due to out-of-bounds access
  (func (export "test_load_out_of_bounds_1") (param i32) (result i64)
    i32.const 65536  ;; memory_size (1 page = 65536 bytes)
    i64.load8_u      ;; This should trap - accessing beyond allocated memory
  )

  ;; Test function: Attempt load far beyond memory bounds (should trap)
  ;; Expected behavior: WebAssembly trap due to out-of-bounds access
  (func (export "test_load_out_of_bounds_2") (param i32) (result i64)
    i32.const 65636  ;; memory_size + 100
    i64.load8_u      ;; This should trap - far beyond allocated memory
  )

  ;; Test function: Attempt load at maximum address value (should trap)
  ;; Expected behavior: WebAssembly trap due to out-of-bounds access
  (func (export "test_load_max_address") (param i32) (result i64)
    i32.const 0xFFFFFFFF  ;; Maximum 32-bit address value
    i64.load8_u           ;; This should trap - way beyond any reasonable memory
  )

  ;; Additional validation function: Test with dynamic address parameter
  ;; This function accepts the address as a parameter to test dynamic addressing
  (func (export "test_load_dynamic") (param i32) (result i64)
    local.get 0
    i64.load8_u
  )

  ;; Stack behavior test function: This function tests i64.load8_u with proper stack setup
  ;; Validates proper stack manipulation and value loading
  (func (export "test_stack_behavior") (param i32) (result i64)
    ;; Push address onto stack
    i32.const 0
    ;; Perform i64.load8_u operation
    i64.load8_u
    ;; Result should be 0x0000000000000042 (zero-extended value at address 0)
  )

  ;; Zero extension validation function: Load 0xFF and verify it becomes 255, not -1
  ;; This function helps verify the difference between signed and unsigned load behavior
  (func (export "test_zero_extension_verification") (param i32) (result i64)
    i32.const 7  ;; Address containing 0xFF
    i64.load8_u  ;; Should produce 0x00000000000000FF (255), not 0xFFFFFFFFFFFFFFFF (-1)
  )

  ;; Boundary value comprehensive test: Load various boundary values in sequence
  ;; This function tests multiple boundary conditions in a single call
  (func (export "test_boundary_sequence") (param i32) (result i64)
    ;; Test sequence: 0x00, 0x7F, 0x80, 0xFF
    ;; Each should be zero-extended properly
    local.get 0
    i32.const 4  ;; Base address for boundary values (0x00)
    i32.add
    i64.load8_u
  )
)