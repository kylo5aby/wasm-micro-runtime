(module
  ;; Memory with 1 page (64KB) for i32.load testing
  (memory (export "memory") 1)

  ;; Data section to initialize memory with test patterns
  (data (i32.const 0) "\78\56\34\12")    ;; 0x12345678 at address 0 (little-endian)
  (data (i32.const 4) "\21\43\65\87")    ;; 0x87654321 at address 4 (little-endian)
  (data (i32.const 8) "\00\ef\cd\ab")    ;; 0xABCDEF00 at address 8 (little-endian)
  (data (i32.const 12) "\ff\ff\ff\ff")   ;; 0xFFFFFFFF at address 12
  (data (i32.const 16) "\98\ba\dc\fe")   ;; 0xFEDCBA98 at address 16 (little-endian)
  (data (i32.const 20) "\11\22\33\44")   ;; 0x44332211 at address 20 (little-endian)
  (data (i32.const 24) "\55\66\77\88")   ;; 0x88776655 at address 24 (little-endian)
  (data (i32.const 28) "\99\aa\bb\cc")   ;; 0xCCBBAA99 at address 28 (little-endian)
  (data (i32.const 32) "\00\00\00\00")   ;; 0x00000000 at address 32
  (data (i32.const 36) "\ff\ff\ff\ff")   ;; 0xFFFFFFFF at address 36
  (data (i32.const 40) "\de\ad\be\ef")   ;; 0xEFBEADDE at address 40 (little-endian)

  ;; Initialize memory boundary area (near 64KB limit)
  (data (i32.const 65532) "\12\34\56\78") ;; Test data at last valid address

  ;; Basic i32.load function - loads 32-bit value from given address
  ;; Parameters: address (i32)
  ;; Returns: loaded value (i32)
  (func (export "test_i32_load") (param $addr i32) (result i32)
    (i32.load (local.get $addr))
  )

  ;; i32.load with offset=4 - loads from address + 4
  ;; Parameters: base address (i32)
  ;; Returns: loaded value from address + 4 (i32)
  (func (export "test_i32_load_offset4") (param $addr i32) (result i32)
    (i32.load offset=4 (local.get $addr))
  )

  ;; i32.load with offset=8 - loads from address + 8
  ;; Parameters: base address (i32)
  ;; Returns: loaded value from address + 8 (i32)
  (func (export "test_i32_load_offset8") (param $addr i32) (result i32)
    (i32.load offset=8 (local.get $addr))
  )

  ;; i32.load with offset=16 - loads from address + 16
  ;; Parameters: base address (i32)
  ;; Returns: loaded value from address + 16 (i32)
  (func (export "test_i32_load_offset16") (param $addr i32) (result i32)
    (i32.load offset=16 (local.get $addr))
  )

  ;; i32.load with 1-byte alignment hint (align=0)
  ;; Parameters: address (i32)
  ;; Returns: loaded value (i32)
  (func (export "test_i32_load_align1") (param $addr i32) (result i32)
    (i32.load align=1 (local.get $addr))
  )

  ;; i32.load with 2-byte alignment hint (align=1)
  ;; Parameters: address (i32)
  ;; Returns: loaded value (i32)
  (func (export "test_i32_load_align2") (param $addr i32) (result i32)
    (i32.load align=2 (local.get $addr))
  )

  ;; i32.load with 4-byte alignment hint (align=2)
  ;; Parameters: address (i32)
  ;; Returns: loaded value (i32)
  (func (export "test_i32_load_align4") (param $addr i32) (result i32)
    (i32.load align=4 (local.get $addr))
  )

  ;; Complex test function that loads and manipulates multiple values
  ;; Demonstrates i32.load in computation context
  ;; Parameters: base address (i32)
  ;; Returns: sum of values at consecutive addresses (i32)
  (func (export "test_i32_load_computation") (param $addr i32) (result i32)
    (i32.add
      (i32.load (local.get $addr))          ;; Load from base address
      (i32.load offset=4 (local.get $addr)) ;; Load from base + 4
    )
  )

  ;; Memory boundary test function - loads from specific boundary addresses
  ;; Tests loading from addresses near memory limits
  ;; Parameters: address (i32)
  ;; Returns: loaded value (i32)
  (func (export "test_boundary_load") (param $addr i32) (result i32)
    (i32.load (local.get $addr))
  )

  ;; Stack underflow test preparation function
  ;; Creates a scenario where stack operations can be tested
  ;; Parameters: address (i32)
  ;; Returns: result after stack manipulation (i32)
  (func (export "test_stack_operations") (param $addr i32) (result i32)
    (local $temp i32)
    (local.set $temp (i32.load (local.get $addr)))
    (local.get $temp)
  )

  ;; Endianness validation function
  ;; Loads a multi-byte value to verify little-endian interpretation
  ;; Parameters: address (i32)
  ;; Returns: loaded value for endianness check (i32)
  (func (export "test_endianness") (param $addr i32) (result i32)
    (i32.load (local.get $addr))
  )

  ;; Multi-load test function - performs multiple consecutive loads
  ;; Tests memory access patterns and caching behavior
  ;; Parameters: base address (i32)
  ;; Returns: XOR of multiple loaded values (i32)
  (func (export "test_multi_load") (param $addr i32) (result i32)
    (i32.xor
      (i32.xor
        (i32.load (local.get $addr))
        (i32.load offset=4 (local.get $addr))
      )
      (i32.load offset=8 (local.get $addr))
    )
  )

  ;; Error condition test helper - attempts load at potentially invalid address
  ;; Used by C++ tests to verify trap generation
  ;; Parameters: address (i32)
  ;; Returns: loaded value or traps (i32)
  (func (export "test_error_conditions") (param $addr i32) (result i32)
    (i32.load (local.get $addr))
  )

  ;; Memory initialization helper function
  ;; Stores a specific pattern at given address for dynamic test setup
  ;; Parameters: address (i32), value (i32)
  ;; Returns: address for verification (i32)
  (func (export "store_test_value") (param $addr i32) (param $value i32) (result i32)
    (i32.store (local.get $addr) (local.get $value))
    (local.get $addr)
  )

  ;; Load-store round-trip test function
  ;; Stores a value and immediately loads it back for verification
  ;; Parameters: address (i32), value (i32)
  ;; Returns: loaded value after store (i32)
  (func (export "test_load_store_roundtrip") (param $addr i32) (param $value i32) (result i32)
    (i32.store (local.get $addr) (local.get $value))
    (i32.load (local.get $addr))
  )

  ;; Memory pattern verification function
  ;; Loads and verifies specific bit patterns in memory
  ;; Parameters: address (i32)
  ;; Returns: loaded pattern for verification (i32)
  (func (export "test_bit_patterns") (param $addr i32) (result i32)
    (i32.load (local.get $addr))
  )

  ;; Advanced offset calculation test
  ;; Tests complex address calculations with large offsets
  ;; Parameters: base address (i32)
  ;; Returns: loaded value from calculated address (i32)
  (func (export "test_large_offset") (param $addr i32) (result i32)
    (i32.load offset=1024 (local.get $addr))
  )
)