(module
  ;; Memory section - 1024 bytes (16 pages * 64KB per page)
  (memory $mem 16)

  ;; Initialize memory with test patterns for v128.load testing
  (data (i32.const 0)
    ;; Sequential pattern at address 0 (16 bytes)
    "\00\01\02\03\04\05\06\07\08\09\0a\0b\0c\0d\0e\0f"
    ;; Additional padding to reach address 64
    "\10\11\12\13\14\15\16\17\18\19\1a\1b\1c\1d\1e\1f"
    "\20\21\22\23\24\25\26\27\28\29\2a\2b\2c\2d\2e\2f"
    "\30\31\32\33\34\35\36\37\38\39\3a\3b\3c\3d\3e\3f")

  (data (i32.const 64)
    ;; Alternating pattern at address 64 (16 bytes)
    "\aa\55\aa\55\aa\55\aa\55\aa\55\aa\55\aa\55\aa\55")

  (data (i32.const 1008)
    ;; Boundary pattern at address 1008 (last valid 16 bytes)
    "\ff\fe\fd\fc\fb\fa\f9\f8\f7\f6\f5\f4\f3\f2\f1\f0")

  ;; Test function: Basic v128.load with sequential pattern
  (func $test_basic_load (param $addr i32) (result v128)
    local.get $addr
    v128.load)

  ;; Test function: v128.load with alternating pattern
  (func $test_alternating_load (param $addr i32) (result v128)
    local.get $addr
    v128.load)

  ;; Test function: v128.load from boundary position
  (func $test_boundary_load (param $addr i32) (result v128)
    local.get $addr
    v128.load)

  ;; Test function: v128.load from zero address with custom pattern
  (func $test_zero_address_load (param $addr i32) (result v128)
    ;; First set custom pattern at address 0
    i32.const 0
    i32.const 0x20304010  ;; Little endian: 0x10, 0x20, 0x30, 0x40
    i32.store

    i32.const 4
    i32.const 0x60704050  ;; Little endian: 0x50, 0x60, 0x70, 0x80
    i32.store

    i32.const 8
    i32.const 0xa0b09090  ;; Little endian: 0x90, 0x90, 0xb0, 0xa0
    i32.store

    i32.const 12
    i32.const 0x00f0e0d0  ;; Little endian: 0xd0, 0xe0, 0xf0, 0x00
    i32.store

    ;; Now load v128 from address 0
    local.get $addr
    v128.load)

  ;; Test function: Generic load function for error testing
  (func $test_load_function (param $addr i32) (result v128)
    local.get $addr
    v128.load)

  ;; Export all test functions
  (export "test_basic_load" (func $test_basic_load))
  (export "test_alternating_load" (func $test_alternating_load))
  (export "test_boundary_load" (func $test_boundary_load))
  (export "test_zero_address_load" (func $test_zero_address_load))
  (export "test_load_function" (func $test_load_function))
)