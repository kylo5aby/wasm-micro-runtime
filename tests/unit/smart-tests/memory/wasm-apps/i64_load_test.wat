(module
  ;; Memory with 1 page (64KB) for i64.load testing
  (memory (export "memory") 1)

  ;; Data section to initialize memory with test patterns for i64.load
  ;; All values stored in little-endian format

  ;; Address 0: 0x123456789ABCDEF0 (8 bytes)
  (data (i32.const 0) "\f0\de\bc\9a\78\56\34\12")

  ;; Address 8: 0x0FEDCBA987654321 (8 bytes)
  (data (i32.const 8) "\21\43\65\87\a9\cb\ed\0f")

  ;; Address 16: 0xAAAA555500001111 (8 bytes)
  (data (i32.const 16) "\11\11\00\00\55\55\aa\aa")

  ;; Address 24: 0x1122334455667788 (8 bytes)
  (data (i32.const 24) "\88\77\66\55\44\33\22\11")

  ;; Address 32: 0x0000000000000000 (zero pattern)
  (data (i32.const 32) "\00\00\00\00\00\00\00\00")

  ;; Address 40: 0xFFFFFFFFFFFFFFFF (all-ones pattern)
  (data (i32.const 40) "\ff\ff\ff\ff\ff\ff\ff\ff")

  ;; Address 48: 0x7FFFFFFFFFFFFFFF (maximum positive signed i64)
  (data (i32.const 48) "\ff\ff\ff\ff\ff\ff\ff\7f")

  ;; Address 56: 0x8000000000000000 (minimum negative signed i64)
  (data (i32.const 56) "\00\00\00\00\00\00\00\80")

  ;; Address 64: 0x0123456789ABCDEF (recognizable pattern)
  (data (i32.const 64) "\ef\cd\ab\89\67\45\23\01")

  ;; Address 72: 0x5555555555555555 (alternating pattern 1)
  (data (i32.const 72) "\55\55\55\55\55\55\55\55")

  ;; Address 80: 0xAAAAAAAAAAAAAAAA (alternating pattern 2)
  (data (i32.const 80) "\aa\aa\aa\aa\aa\aa\aa\aa")

  ;; Initialize memory boundary area (near 64KB limit)
  ;; Address 65528: Last valid 8-byte boundary (65536 - 8)
  (data (i32.const 65528) "\12\34\56\78\9a\bc\de\f0")

  ;; Basic i64.load function - loads 64-bit value from given address
  ;; Parameters: address (i32)
  ;; Returns: loaded value (i64)
  (func (export "test_i64_load") (param $addr i32) (result i64)
    (i64.load (local.get $addr))
  )

  ;; i64.load with offset=8 - loads from address + 8
  ;; Parameters: base address (i32)
  ;; Returns: loaded value from address + 8 (i64)
  (func (export "test_i64_load_offset8") (param $addr i32) (result i64)
    (i64.load offset=8 (local.get $addr))
  )

  ;; i64.load with offset=16 - loads from address + 16
  ;; Parameters: base address (i32)
  ;; Returns: loaded value from address + 16 (i64)
  (func (export "test_i64_load_offset16") (param $addr i32) (result i64)
    (i64.load offset=16 (local.get $addr))
  )

  ;; i64.load with offset=32 - loads from address + 32
  ;; Parameters: base address (i32)
  ;; Returns: loaded value from address + 32 (i64)
  (func (export "test_i64_load_offset32") (param $addr i32) (result i64)
    (i64.load offset=32 (local.get $addr))
  )

  ;; i64.load with 1-byte alignment hint (align=0)
  ;; Parameters: address (i32)
  ;; Returns: loaded value (i64)
  (func (export "test_i64_load_align1") (param $addr i32) (result i64)
    (i64.load align=1 (local.get $addr))
  )

  ;; i64.load with 2-byte alignment hint (align=1)
  ;; Parameters: address (i32)
  ;; Returns: loaded value (i64)
  (func (export "test_i64_load_align2") (param $addr i32) (result i64)
    (i64.load align=2 (local.get $addr))
  )

  ;; i64.load with 4-byte alignment hint (align=2)
  ;; Parameters: address (i32)
  ;; Returns: loaded value (i64)
  (func (export "test_i64_load_align4") (param $addr i32) (result i64)
    (i64.load align=4 (local.get $addr))
  )

  ;; i64.load with 8-byte alignment hint (align=3)
  ;; Parameters: address (i32)
  ;; Returns: loaded value (i64)
  (func (export "test_i64_load_align8") (param $addr i32) (result i64)
    (i64.load align=8 (local.get $addr))
  )

  ;; Complex test function that loads and manipulates multiple 64-bit values
  ;; Demonstrates i64.load in computation context
  ;; Parameters: base address (i32)
  ;; Returns: XOR of values at consecutive addresses (i64)
  (func (export "test_i64_load_computation") (param $addr i32) (result i64)
    (i64.xor
      (i64.load (local.get $addr))          ;; Load from base address
      (i64.load offset=8 (local.get $addr)) ;; Load from base + 8
    )
  )

  ;; Memory boundary test function - loads from specific boundary addresses
  ;; Tests loading from addresses near memory limits
  ;; Parameters: address (i32)
  ;; Returns: loaded value (i64)
  (func (export "test_boundary_load") (param $addr i32) (result i64)
    (i64.load (local.get $addr))
  )

  ;; Stack manipulation test function
  ;; Creates a scenario where stack operations with i64 can be tested
  ;; Parameters: address (i32)
  ;; Returns: result after stack manipulation (i64)
  (func (export "test_stack_operations") (param $addr i32) (result i64)
    (local $temp i64)
    (local.set $temp (i64.load (local.get $addr)))
    (local.get $temp)
  )

  ;; Endianness validation function for 64-bit values
  ;; Loads a multi-byte value to verify little-endian interpretation
  ;; Parameters: address (i32)
  ;; Returns: loaded value for endianness check (i64)
  (func (export "test_endianness") (param $addr i32) (result i64)
    (i64.load (local.get $addr))
  )

  ;; Multi-load test function - performs multiple consecutive i64 loads
  ;; Tests memory access patterns and caching behavior with 64-bit values
  ;; Parameters: base address (i32)
  ;; Returns: XOR of multiple loaded 64-bit values (i64)
  (func (export "test_multi_load") (param $addr i32) (result i64)
    (i64.xor
      (i64.xor
        (i64.load (local.get $addr))
        (i64.load offset=8 (local.get $addr))
      )
      (i64.load offset=16 (local.get $addr))
    )
  )

  ;; Error condition test helper - attempts load at potentially invalid address
  ;; Used by C++ tests to verify trap generation for i64.load
  ;; Parameters: address (i32)
  ;; Returns: loaded value or traps (i64)
  (func (export "test_error_conditions") (param $addr i32) (result i64)
    (i64.load (local.get $addr))
  )

  ;; Memory initialization helper function for 64-bit values
  ;; Stores a specific 64-bit pattern at given address for dynamic test setup
  ;; Parameters: address (i32), value (i64)
  ;; Returns: address for verification (i32)
  (func (export "store_test_value") (param $addr i32) (param $value i64) (result i32)
    (i64.store (local.get $addr) (local.get $value))
    (local.get $addr)
  )

  ;; Load-store round-trip test function for 64-bit values
  ;; Stores a 64-bit value and immediately loads it back for verification
  ;; Parameters: address (i32), value (i64)
  ;; Returns: loaded value after store (i64)
  (func (export "test_load_store_roundtrip") (param $addr i32) (param $value i64) (result i64)
    (i64.store (local.get $addr) (local.get $value))
    (i64.load (local.get $addr))
  )

  ;; Memory pattern verification function for 64-bit patterns
  ;; Loads and verifies specific bit patterns in memory
  ;; Parameters: address (i32)
  ;; Returns: loaded pattern for verification (i64)
  (func (export "test_bit_patterns") (param $addr i32) (result i64)
    (i64.load (local.get $addr))
  )

  ;; Unaligned access test function
  ;; Tests i64.load from unaligned memory addresses
  ;; Parameters: address (i32)
  ;; Returns: loaded value from unaligned address (i64)
  (func (export "test_unaligned_load") (param $addr i32) (result i64)
    (i64.load (local.get $addr))
  )

  ;; Large offset calculation test
  ;; Tests complex address calculations with large offsets for i64
  ;; Parameters: base address (i32)
  ;; Returns: loaded value from calculated address (i64)
  (func (export "test_large_offset") (param $addr i32) (result i64)
    (i64.load offset=1024 (local.get $addr))
  )

  ;; Cross-page boundary test function
  ;; Tests i64.load operations that span virtual memory page boundaries
  ;; Parameters: address (i32)
  ;; Returns: loaded value crossing page boundaries (i64)
  (func (export "test_cross_page_load") (param $addr i32) (result i64)
    (i64.load (local.get $addr))
  )

  ;; Maximum positive signed value test
  ;; Loads and returns the maximum positive signed i64 value
  ;; Parameters: none
  ;; Returns: 0x7FFFFFFFFFFFFFFF (i64)
  (func (export "test_max_positive") (result i64)
    (i64.load (i32.const 48))
  )

  ;; Minimum negative signed value test
  ;; Loads and returns the minimum negative signed i64 value
  ;; Parameters: none
  ;; Returns: 0x8000000000000000 (i64)
  (func (export "test_min_negative") (result i64)
    (i64.load (i32.const 56))
  )

  ;; Zero value test
  ;; Loads and returns zero from memory
  ;; Parameters: none
  ;; Returns: 0x0000000000000000 (i64)
  (func (export "test_zero_value") (result i64)
    (i64.load (i32.const 32))
  )

  ;; Maximum unsigned value test
  ;; Loads and returns the maximum unsigned i64 value
  ;; Parameters: none
  ;; Returns: 0xFFFFFFFFFFFFFFFF (i64)
  (func (export "test_max_unsigned") (result i64)
    (i64.load (i32.const 40))
  )
)