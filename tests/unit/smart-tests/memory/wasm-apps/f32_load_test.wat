(module
  ;; Memory with 1 page (64KB) for f32.load testing
  (memory (export "memory") 1)

  ;; Data section to initialize memory with IEEE 754 float test patterns
  ;; All values are in little-endian format (least significant byte first)

  ;; Basic test values
  (data (i32.const 0) "\c3\f5\48\40")    ;; 3.14f (0x40485FC3) at address 0
  (data (i32.const 4) "\00\00\20\c0")    ;; -2.5f (0xC0200000) at address 4
  (data (i32.const 8) "\00\00\80\3f")    ;; 1.0f (0x3F800000) at address 8
  (data (i32.const 12) "\00\00\00\00")   ;; 0.0f (0x00000000) at address 12

  ;; Boundary test values
  (data (i32.const 16) "\00\00\c8\c2")   ;; -100.0f (0xC2C80000) at address 16
  (data (i32.const 20) "\00\00\c8\42")   ;; 100.0f (0x42C80000) at address 20
  (data (i32.const 24) "\00\00\80\44")   ;; 1024.0f (0x44800000) at address 24
  (data (i32.const 28) "\00\00\80\c4")   ;; -1024.0f (0xC4800000) at address 28

  ;; Offset test values
  (data (i32.const 32) "\6f\12\83\3a")   ;; 0.001f (0x3A83126F) at address 32
  (data (i32.const 36) "\00\00\7a\44")   ;; 1000.0f (0x447A0000) at address 36
  (data (i32.const 40) "\00\00\48\43")   ;; 200.0f (0x43480000) at address 40
  (data (i32.const 44) "\a6\9b\44\3b")   ;; 0.003f (0x3B449BA6) at address 44

  ;; Large value test
  (data (i32.const 48) "\52\b8\1e\45")   ;; 2539.32f (0x451EB852) at address 48
  (data (i32.const 52) "\d7\a3\20\45")   ;; 2570.24f (0x4520A3D7) at address 52
  (data (i32.const 56) "\00\80\99\47")   ;; 78336.0f (0x47998000) at address 56
  (data (i32.const 60) "\9a\f9\79\44")   ;; 999.9f (0x4479F99A) at address 60

  ;; IEEE 754 special values for comprehensive testing
  (data (i32.const 64) "\00\00\00\00")   ;; +0.0f (0x00000000) at address 64
  (data (i32.const 68) "\00\00\00\80")   ;; -0.0f (0x80000000) at address 68
  (data (i32.const 72) "\00\00\80\7f")   ;; +∞ (0x7F800000) at address 72
  (data (i32.const 76) "\00\00\80\ff")   ;; -∞ (0xFF800000) at address 76
  (data (i32.const 80) "\00\00\c0\7f")   ;; Quiet NaN (0x7FC00000) at address 80
  (data (i32.const 84) "\01\00\80\7f")   ;; Signaling NaN (0x7F800001) at address 84

  ;; Denormal/subnormal values
  (data (i32.const 88) "\01\00\00\00")   ;; Minimum positive subnormal (0x00000001) at address 88
  (data (i32.const 92) "\ff\ff\7f\00")   ;; Maximum subnormal (0x007FFFFF) at address 92
  (data (i32.const 96) "\01\00\00\80")   ;; Minimum negative subnormal (0x80000001) at address 96
  (data (i32.const 100) "\ff\ff\7f\80")  ;; Maximum negative subnormal (0x807FFFFF) at address 100

  ;; Extreme normal values
  (data (i32.const 104) "\00\00\80\00")  ;; Minimum positive normal (0x00800000) at address 104
  (data (i32.const 108) "\ff\ff\7f\7f")  ;; Maximum finite (0x7F7FFFFF) at address 108
  (data (i32.const 112) "\00\00\80\80")  ;; Maximum negative normal (0x80800000) at address 112
  (data (i32.const 116) "\ff\ff\7f\ff")  ;; Minimum finite (0xFF7FFFFF) at address 116

  ;; Basic f32.load function - loads 32-bit float from given address
  ;; Parameters: address (i32)
  ;; Returns: loaded float value as i32 bits (f32)
  (func (export "test_f32_load") (param $addr i32) (result f32)
    (f32.load (local.get $addr))
  )

  ;; f32.load with offset=4 - loads from address + 4
  ;; Parameters: base address (i32)
  ;; Returns: loaded float from address + 4 (f32)
  (func (export "test_f32_load_offset4") (param $addr i32) (result f32)
    (f32.load offset=4 (local.get $addr))
  )

  ;; f32.load with offset=8 - loads from address + 8
  ;; Parameters: base address (i32)
  ;; Returns: loaded float from address + 8 (f32)
  (func (export "test_f32_load_offset8") (param $addr i32) (result f32)
    (f32.load offset=8 (local.get $addr))
  )

  ;; f32.load with offset=16 - loads from address + 16
  ;; Parameters: base address (i32)
  ;; Returns: loaded float from address + 16 (f32)
  (func (export "test_f32_load_offset16") (param $addr i32) (result f32)
    (f32.load offset=16 (local.get $addr))
  )

  ;; f32.load with offset=32 - loads from address + 32
  ;; Parameters: base address (i32)
  ;; Returns: loaded float from address + 32 (f32)
  (func (export "test_f32_load_offset32") (param $addr i32) (result f32)
    (f32.load offset=32 (local.get $addr))
  )

  ;; f32.load with 4-byte alignment (natural alignment for f32)
  ;; Parameters: address (i32)
  ;; Returns: loaded float value (f32)
  (func (export "test_f32_load_aligned") (param $addr i32) (result f32)
    (f32.load align=4 (local.get $addr))
  )

  ;; f32.load with unaligned access (1-byte alignment)
  ;; Parameters: address (i32)
  ;; Returns: loaded float value (f32)
  (func (export "test_f32_load_unaligned") (param $addr i32) (result f32)
    (f32.load align=1 (local.get $addr))
  )

  ;; Store helper function to set up test data at runtime
  ;; Parameters: address (i32), value (f32)
  ;; Returns: none
  (func (export "store_f32_at_address") (param $addr i32) (param $value f32)
    (f32.store (local.get $addr) (local.get $value))
  )

  ;; Combined load and store test function
  ;; Parameters: source address (i32), destination address (i32)
  ;; Returns: value that was copied (f32)
  (func (export "test_f32_copy") (param $src i32) (param $dst i32) (result f32)
    (local $value f32)
    (local.set $value (f32.load (local.get $src)))
    (f32.store (local.get $dst) (local.get $value))
    (local.get $value)
  )

  ;; Memory initialization function for runtime setup
  ;; Sets up additional test values not covered by data sections
  (func (export "initialize_test_memory")
    ;; Store some runtime-calculated values
    (f32.store (i32.const 200) (f32.const 42.42))
    (f32.store (i32.const 204) (f32.const -42.42))
    (f32.store (i32.const 208) (f32.const 0.5))
    (f32.store (i32.const 212) (f32.const -0.5))
  )

  ;; Memory bounds validation function
  ;; Parameters: address (i32)
  ;; Returns: 1 if within bounds, 0 if would cause trap
  (func (export "check_bounds") (param $addr i32) (result i32)
    (if (result i32) (i32.gt_u (local.get $addr) (i32.const 65532))
      (then (i32.const 0))  ;; Out of bounds
      (else (i32.const 1))  ;; Within bounds
    )
  )

  ;; Get memory size in pages
  ;; Returns: number of pages (i32)
  (func (export "get_memory_pages") (result i32)
    (memory.size)
  )

  ;; Get memory size in bytes
  ;; Returns: number of bytes (i32)
  (func (export "get_memory_bytes") (result i32)
    (i32.mul (memory.size) (i32.const 65536))
  )
)