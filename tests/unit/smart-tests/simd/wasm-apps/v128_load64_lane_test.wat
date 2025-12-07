(module
  ;; Memory section with initial test data
  (memory (export "memory") 2)

  ;; Initialize memory with test data patterns
  (data (i32.const 0)
    ;; Address 0: 0x123456789ABCDEF0 (little-endian)
    "\f0\de\bc\9a\78\56\34\12"
    ;; Address 8: 0xFEDCBA0987654321 (little-endian)
    "\21\43\65\87\09\ba\dc\fe"
    ;; Address 16: 0x0011223344556677 (little-endian)
    "\77\66\55\44\33\22\11\00"
    ;; Address 24: 0x8899AABBCCDDEEFF (little-endian)
    "\ff\ee\dd\cc\bb\aa\99\88"
    ;; Address 32: 0x0000000000000000 (zero value)
    "\00\00\00\00\00\00\00\00"
    ;; Address 40: 0xFFFFFFFFFFFFFFFF (max value)
    "\ff\ff\ff\ff\ff\ff\ff\ff"
    ;; Address 48: 0x0102030405060708 (endianness test pattern)
    "\08\07\06\05\04\03\02\01"
    ;; Address 56: 0xF0F1F2F3F4F5F6F7 (boundary test pattern)
    "\f7\f6\f5\f4\f3\f2\f1\f0"
    ;; Additional padding for boundary tests
    "\00\00\00\00\00\00\00\00"
  )

  ;; Test function for v128.load64_lane with lane index 0
  (func (export "test_v128_load64_lane_0")
    (param $base_addr i32)
    (param $input_lane0_low i32)
    (param $input_lane0_high i32)
    (param $input_lane1_low i32)
    (result i32 i32 i32 i32)
    (local $result v128)

    ;; Create input v128 vector with known pattern
    (v128.const i64x2 0x5555555555555555 0xAAAAAAAAAAAAAAAA)
    (local.set $result)

    ;; Load 64-bit value from memory at base_addr into lane 0
    (local.get $base_addr)
    (local.get $result)
    (v128.load64_lane 0)
    (local.set $result)

    ;; Extract lanes for return (return as 4 i32 values)
    ;; Extract lane 0 (low 32 bits)
    (local.get $result)
    (i64x2.extract_lane 0)
    (i32.wrap_i64)

    ;; Extract lane 0 (high 32 bits)
    (local.get $result)
    (i64x2.extract_lane 0)
    (i64.const 32)
    (i64.shr_u)
    (i32.wrap_i64)

    ;; Extract lane 1 (low 32 bits)
    (local.get $result)
    (i64x2.extract_lane 1)
    (i32.wrap_i64)

    ;; Extract lane 1 (high 32 bits)
    (local.get $result)
    (i64x2.extract_lane 1)
    (i64.const 32)
    (i64.shr_u)
    (i32.wrap_i64)
  )

  ;; Test function for v128.load64_lane with lane index 1
  (func (export "test_v128_load64_lane_1")
    (param $base_addr i32)
    (param $input_lane0_low i32)
    (param $input_lane0_high i32)
    (param $input_lane1_low i32)
    (result i32 i32 i32 i32)
    (local $result v128)

    ;; Create base v128 vector with known pattern
    (v128.const i64x2 0x1111111111111111 0x9999999999999999)
    (local.set $result)

    ;; Load 64-bit value from memory at base_addr into lane 1
    (local.get $base_addr)
    (local.get $result)
    (v128.load64_lane 1)
    (local.set $result)

    ;; Extract lanes for return (return as 4 i32 values)
    ;; Extract lane 0 (low 32 bits)
    (local.get $result)
    (i64x2.extract_lane 0)
    (i32.wrap_i64)

    ;; Extract lane 0 (high 32 bits)
    (local.get $result)
    (i64x2.extract_lane 0)
    (i64.const 32)
    (i64.shr_u)
    (i32.wrap_i64)

    ;; Extract lane 1 (low 32 bits)
    (local.get $result)
    (i64x2.extract_lane 1)
    (i32.wrap_i64)

    ;; Extract lane 1 (high 32 bits)
    (local.get $result)
    (i64x2.extract_lane 1)
    (i64.const 32)
    (i64.shr_u)
    (i32.wrap_i64)
  )

  ;; Test function for boundary access - lane 0 with offset
  (func (export "test_v128_load64_lane_offset_0")
    (param $base_addr i32)
    (result i32 i32 i32 i32)
    (local $result v128)

    ;; Create base v128 vector
    (v128.const i64x2 0x2222222222222222 0x4444444444444444)
    (local.set $result)

    ;; Load with offset (memarg offset=8)
    (local.get $base_addr)
    (local.get $result)
    (v128.load64_lane offset=8 0)
    (local.set $result)

    ;; Extract and return lanes
    (local.get $result)
    (i64x2.extract_lane 0)
    (i32.wrap_i64)

    (local.get $result)
    (i64x2.extract_lane 0)
    (i64.const 32)
    (i64.shr_u)
    (i32.wrap_i64)

    (local.get $result)
    (i64x2.extract_lane 1)
    (i32.wrap_i64)

    (local.get $result)
    (i64x2.extract_lane 1)
    (i64.const 32)
    (i64.shr_u)
    (i32.wrap_i64)
  )

  ;; Test function for alignment testing - unaligned access
  (func (export "test_v128_load64_lane_unaligned")
    (param $base_addr i32)
    (result i32 i32 i32 i32)
    (local $result v128)

    ;; Create base v128 vector
    (v128.const i64x2 0x3333333333333333 0x6666666666666666)
    (local.set $result)

    ;; Load from unaligned address (add 4 to make it 4-byte but not 8-byte aligned)
    (local.get $base_addr)
    (i32.const 4)
    (i32.add)
    (local.get $result)
    (v128.load64_lane align=4 1)
    (local.set $result)

    ;; Extract and return lanes
    (local.get $result)
    (i64x2.extract_lane 0)
    (i32.wrap_i64)

    (local.get $result)
    (i64x2.extract_lane 0)
    (i64.const 32)
    (i64.shr_u)
    (i32.wrap_i64)

    (local.get $result)
    (i64x2.extract_lane 1)
    (i32.wrap_i64)

    (local.get $result)
    (i64x2.extract_lane 1)
    (i64.const 32)
    (i64.shr_u)
    (i32.wrap_i64)
  )

  ;; Test function for zero base address
  (func (export "test_v128_load64_lane_zero_addr")
    (result i32 i32 i32 i32)
    (local $result v128)

    ;; Create base v128 vector
    (v128.const i64x2 0x7777777777777777 0x8888888888888888)
    (local.set $result)

    ;; Load from address 0
    (i32.const 0)
    (local.get $result)
    (v128.load64_lane 0)
    (local.set $result)

    ;; Extract and return lanes
    (local.get $result)
    (i64x2.extract_lane 0)
    (i32.wrap_i64)

    (local.get $result)
    (i64x2.extract_lane 0)
    (i64.const 32)
    (i64.shr_u)
    (i32.wrap_i64)

    (local.get $result)
    (i64x2.extract_lane 1)
    (i32.wrap_i64)

    (local.get $result)
    (i64x2.extract_lane 1)
    (i64.const 32)
    (i64.shr_u)
    (i32.wrap_i64)
  )

  ;; Helper function to get memory size for boundary testing
  (func (export "get_memory_size") (result i32)
    (memory.size)
    (i32.const 65536)  ;; Convert pages to bytes
    (i32.mul)
  )

  ;; Test function for maximum alignment
  (func (export "test_v128_load64_lane_max_align")
    (param $base_addr i32)
    (result i32 i32 i32 i32)
    (local $result v128)

    ;; Create base v128 vector
    (v128.const i64x2 0xBBBBBBBBBBBBBBBB 0xCCCCCCCCCCCCCCCC)
    (local.set $result)

    ;; Load with maximum alignment (8 bytes)
    (local.get $base_addr)
    (local.get $result)
    (v128.load64_lane align=8 1)
    (local.set $result)

    ;; Extract and return lanes
    (local.get $result)
    (i64x2.extract_lane 0)
    (i32.wrap_i64)

    (local.get $result)
    (i64x2.extract_lane 0)
    (i64.const 32)
    (i64.shr_u)
    (i32.wrap_i64)

    (local.get $result)
    (i64x2.extract_lane 1)
    (i32.wrap_i64)

    (local.get $result)
    (i64x2.extract_lane 1)
    (i64.const 32)
    (i64.shr_u)
    (i32.wrap_i64)
  )
)