(module
  ;; Memory with 1 page (64KB) for i64.load32_u testing
  (memory (export "memory") 1)

  ;; Data section to initialize memory with test patterns for i64.load32_u
  ;; All 32-bit values stored in little-endian format

  ;; Address 0: 0x12345678 (typical positive 32-bit value)
  (data (i32.const 0) "\78\56\34\12")

  ;; Address 4: 0xABCDEF00 (high bit pattern)
  (data (i32.const 4) "\00\ef\cd\ab")

  ;; Address 8: 0x80000001 (MSB set - would be negative if sign-extended)
  (data (i32.const 8) "\01\00\00\80")

  ;; Address 12: 0x7FFFFFFF (maximum signed positive, safe for both signed/unsigned)
  (data (i32.const 12) "\ff\ff\ff\7f")

  ;; Address 16: 0x80000000 (MSB set, minimum signed negative but unsigned positive)
  (data (i32.const 16) "\00\00\00\80")

  ;; Address 20: 0x90000000 (MSB set, random pattern)
  (data (i32.const 20) "\00\00\00\90")

  ;; Address 24: 0xFFFFFFFF (maximum u32 value, all bits set)
  (data (i32.const 24) "\ff\ff\ff\ff")

  ;; Address 28: 0x00000000 (u32 minimum value)
  (data (i32.const 28) "\00\00\00\00")

  ;; Address 32: 0xFFFFFFFF (u32 maximum value, duplicate for boundary testing)
  (data (i32.const 32) "\ff\ff\ff\ff")

  ;; Address 36: 0xDEADBEEF (unaligned test pattern for alignment testing)
  (data (i32.const 36) "\ef\be\ad\de")

  ;; Address 41: 0xDEADBEEF (1-byte misaligned)
  (data (i32.const 41) "\ef\be\ad\de")

  ;; Address 46: 0xDEADBEEF (2-byte misaligned)
  (data (i32.const 46) "\ef\be\ad\de")

  ;; Address 51: 0xDEADBEEF (3-byte misaligned)
  (data (i32.const 51) "\ef\be\ad\de")

  ;; Address 60-72: Offset testing patterns
  ;; Address 60: 0x11111111
  (data (i32.const 60) "\11\11\11\11")
  ;; Address 64: 0x22222222
  (data (i32.const 64) "\22\22\22\22")
  ;; Address 68: 0x33333333
  (data (i32.const 68) "\33\33\33\33")
  ;; Address 72: 0x44444444
  (data (i32.const 72) "\44\44\44\44")

  ;; Initialize memory boundary area (near 64KB limit)
  ;; Address 65532: Last valid 4-byte boundary for i64.load32_u (65536 - 4)
  (data (i32.const 65532) "\bb\bb\bb\bb")

  ;; Basic i64.load32_u function - loads unsigned 32-bit value and zero-extends to 64-bit
  ;; Parameters: address (i32)
  ;; Returns: zero-extended value (i64)
  (func $test_i64_load32_u (export "test_i64_load32_u") (param $addr i32) (result i64)
    ;; Load unsigned 32-bit value from memory at $addr and zero-extend to 64-bit
    local.get $addr
    i64.load32_u
  )

  ;; i64.load32_u with static offset function
  ;; Parameters: base_address (i32), offset (i32)
  ;; Returns: zero-extended value (i64)
  ;; Note: The offset parameter is passed but used in the address calculation
  (func $test_i64_load32_u_offset (export "test_i64_load32_u_offset")
        (param $addr i32) (param $offset i32) (result i64)
    ;; Add offset to base address and load
    local.get $addr
    local.get $offset
    i32.add
    i64.load32_u
  )

  ;; Memory initialization helper function
  ;; Parameters: address (i32), value (i32)
  ;; Stores a 32-bit value at the specified memory address
  (func $init_memory_u32 (export "init_memory_u32") (param $addr i32) (param $value i32)
    ;; Store 32-bit value at memory address
    local.get $addr
    local.get $value
    i32.store
  )

  ;; Zero extension verification helper
  ;; Loads a value and returns just the upper 32 bits to verify they are zero
  ;; Parameters: address (i32)
  ;; Returns: upper 32 bits of loaded i64 (i32)
  (func $get_upper_32_bits (export "get_upper_32_bits") (param $addr i32) (result i32)
    ;; Load i64 value and extract upper 32 bits
    local.get $addr
    i64.load32_u
    i64.const 32
    i64.shr_u
    i32.wrap_i64
  )

  ;; Lower 32 bits extraction helper
  ;; Parameters: address (i32)
  ;; Returns: lower 32 bits of loaded i64 (i32)
  (func $get_lower_32_bits (export "get_lower_32_bits") (param $addr i32) (result i32)
    ;; Load i64 value and extract lower 32 bits
    local.get $addr
    i64.load32_u
    i32.wrap_i64
  )

  ;; Compare with signed load variant
  ;; Demonstrates difference between i64.load32_u and i64.load32_s
  ;; Parameters: address (i32)
  ;; Returns: 1 if results differ (MSB set in original), 0 if same (MSB clear)
  (func $compare_signed_unsigned (export "compare_signed_unsigned") (param $addr i32) (result i32)
    (local $unsigned_result i64)
    (local $signed_result i64)

    ;; Load using unsigned extension
    local.get $addr
    i64.load32_u
    local.set $unsigned_result

    ;; Load using signed extension
    local.get $addr
    i64.load32_s
    local.set $signed_result

    ;; Compare results - return 1 if different, 0 if same
    local.get $unsigned_result
    local.get $signed_result
    i64.ne
    i32.const 1
    i32.const 0
    select
  )

  ;; Boundary testing helper - attempts to load from potentially invalid address
  ;; Parameters: address (i32)
  ;; Returns: loaded value (i64) - will trap if address is invalid
  (func $test_boundary_load (export "test_boundary_load") (param $addr i32) (result i64)
    local.get $addr
    i64.load32_u
  )

  ;; Unaligned access test helper
  ;; Parameters: base_address (i32), alignment_offset (i32)
  ;; Returns: value loaded from base + alignment_offset (i64)
  (func $test_unaligned_load (export "test_unaligned_load")
        (param $base i32) (param $align_offset i32) (result i64)
    ;; Calculate unaligned address and load
    local.get $base
    local.get $align_offset
    i32.add
    i64.load32_u
  )

  ;; Static offset demonstration functions with compile-time known offsets

  ;; Load with offset=0 (effectively no offset)
  (func $load_offset_0 (export "load_offset_0") (param $addr i32) (result i64)
    local.get $addr
    i64.load32_u offset=0
  )

  ;; Load with offset=4
  (func $load_offset_4 (export "load_offset_4") (param $addr i32) (result i64)
    local.get $addr
    i64.load32_u offset=4
  )

  ;; Load with offset=8
  (func $load_offset_8 (export "load_offset_8") (param $addr i32) (result i64)
    local.get $addr
    i64.load32_u offset=8
  )

  ;; Load with offset=12
  (func $load_offset_12 (export "load_offset_12") (param $addr i32) (result i64)
    local.get $addr
    i64.load32_u offset=12
  )

  ;; Alignment testing with different align attributes
  ;; Note: align attribute is a hint and doesn't change functionality but may affect performance

  ;; Natural alignment (align=4 for 32-bit access)
  (func $load_aligned_4 (export "load_aligned_4") (param $addr i32) (result i64)
    local.get $addr
    i64.load32_u align=4
  )

  ;; Byte alignment (align=1)
  (func $load_aligned_1 (export "load_aligned_1") (param $addr i32) (result i64)
    local.get $addr
    i64.load32_u align=1
  )

  ;; Memory pattern validation helper
  ;; Verifies that memory contains expected test patterns
  ;; Returns: 1 if patterns are correct, 0 otherwise
  (func $validate_memory_patterns (export "validate_memory_patterns") (result i32)
    (local $valid i32)

    ;; Start with valid=1 (true)
    i32.const 1
    local.set $valid

    ;; Check address 0: should be 0x12345678
    i32.const 0
    i64.load32_u
    i64.const 0x12345678
    i64.ne
    if
      i32.const 0
      local.set $valid
    end

    ;; Check address 24: should be 0xFFFFFFFF
    i32.const 24
    i64.load32_u
    i64.const 0xFFFFFFFF
    i64.ne
    if
      i32.const 0
      local.set $valid
    end

    ;; Check boundary address 65532: should be 0xBBBBBBBB
    i32.const 65532
    i64.load32_u
    i64.const 0xBBBBBBBB
    i64.ne
    if
      i32.const 0
      local.set $valid
    end

    local.get $valid
  )

  ;; Zero extension validation
  ;; Verifies that upper 32 bits are always zero for any loaded value
  ;; Parameters: address (i32)
  ;; Returns: 1 if upper bits are zero, 0 otherwise
  (func $verify_zero_extension (export "verify_zero_extension") (param $addr i32) (result i32)
    ;; Load value and check if upper 32 bits are zero
    local.get $addr
    i64.load32_u
    i64.const 0xFFFFFFFF00000000
    i64.and
    i64.const 0
    i64.eq
    i32.const 1
    i32.const 0
    select
  )

  ;; Maximum value test - loads 0xFFFFFFFF and verifies zero extension
  (func $test_max_u32_value (export "test_max_u32_value") (result i64)
    ;; Load maximum u32 value from address 24
    i32.const 24
    i64.load32_u
  )

  ;; Zero value test - loads 0x00000000 and verifies result
  (func $test_zero_value (export "test_zero_value") (result i64)
    ;; Load zero value from address 28
    i32.const 28
    i64.load32_u
  )
)