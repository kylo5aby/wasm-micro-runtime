(module
  ;; Memory with 1 page (64KB) for f64.load testing
  (memory (export "memory") 1)

  ;; Data section to initialize memory with IEEE 754 double test patterns
  ;; All values are in little-endian format (least significant byte first)

  ;; Basic test values
  (data (i32.const 0) "\18\2d\44\54\fb\21\09\40")    ;; 3.141592653589793 (0x400921FB54442D18) at address 0
  (data (i32.const 8) "\69\57\14\8b\0a\bf\05\c0")    ;; -2.718281828459045 (0xC005BF0A8B145769) at address 8
  (data (i32.const 16) "\00\00\00\00\00\00\f0\3f")   ;; 1.0 (0x3FF0000000000000) at address 16
  (data (i32.const 24) "\00\00\00\00\00\00\00\00")   ;; 0.0 (0x0000000000000000) at address 24

  ;; Boundary test values
  (data (i32.const 32) "\00\00\00\00\00\00\59\c0")   ;; -100.0 (0xC059000000000000) at address 32
  (data (i32.const 40) "\00\00\00\00\00\00\59\40")   ;; 100.0 (0x4059000000000000) at address 40
  (data (i32.const 48) "\00\00\00\00\00\00\90\40")   ;; 1024.0 (0x4090000000000000) at address 48
  (data (i32.const 56) "\00\00\00\00\00\00\90\c0")   ;; -1024.0 (0xC090000000000000) at address 56

  ;; Offset test values
  (data (i32.const 64) "\6e\86\1b\f0\f9\a0\60\3f")   ;; 0.001 (0x3F60A0F9F01B866E) at address 64
  (data (i32.const 72) "\00\00\00\00\00\40\8f\40")   ;; 1000.0 (0x408F400000000000) at address 72
  (data (i32.const 80) "\00\00\00\00\00\00\69\40")   ;; 200.0 (0x4069000000000000) at address 80
  (data (i32.const 88) "\87\16\25\06\81\95\68\3f")   ;; 0.003 (0x3F6895810625168F) at address 88

  ;; Large value test
  (data (i32.const 96) "\83\90\3d\70\a8\d7\a3\40")   ;; 2539.32 (0x40A3D7A8703D9083) at address 96
  (data (i32.const 104) "\fd\87\14\ae\07\28\a4\40")  ;; 2570.24 (0x40A42807AE1487FD) at address 104
  (data (i32.const 112) "\00\00\00\00\00\30\f3\40")  ;; 78336.0 (0x40F3300000000000) at address 112
  (data (i32.const 120) "\33\33\33\33\33\3f\8f\40")  ;; 999.9 (0x408F3F3333333333) at address 120

  ;; IEEE 754 special values for comprehensive testing
  (data (i32.const 128) "\00\00\00\00\00\00\00\00")  ;; +0.0 (0x0000000000000000) at address 128
  (data (i32.const 136) "\00\00\00\00\00\00\00\80")  ;; -0.0 (0x8000000000000000) at address 136
  (data (i32.const 144) "\00\00\00\00\00\00\f0\7f")  ;; +∞ (0x7FF0000000000000) at address 144
  (data (i32.const 152) "\00\00\00\00\00\00\f0\ff")  ;; -∞ (0xFFF0000000000000) at address 152
  (data (i32.const 160) "\00\00\00\00\00\00\f8\7f")  ;; Quiet NaN (0x7FF8000000000000) at address 160
  (data (i32.const 168) "\01\00\00\00\00\00\f0\7f")  ;; Signaling NaN (0x7FF0000000000001) at address 168

  ;; Denormal/subnormal values
  (data (i32.const 176) "\01\00\00\00\00\00\00\00")  ;; Minimum positive subnormal (0x0000000000000001) at address 176
  (data (i32.const 184) "\ff\ff\ff\ff\ff\ff\0f\00")  ;; Maximum subnormal (0x000FFFFFFFFFFFFF) at address 184
  (data (i32.const 192) "\01\00\00\00\00\00\00\80")  ;; Minimum negative subnormal (0x8000000000000001) at address 192
  (data (i32.const 200) "\ff\ff\ff\ff\ff\ff\0f\80")  ;; Maximum negative subnormal (0x800FFFFFFFFFFFFF) at address 200

  ;; Extreme normal values
  (data (i32.const 208) "\00\00\00\00\00\00\10\00")  ;; Minimum positive normal (0x0010000000000000) at address 208
  (data (i32.const 216) "\ff\ff\ff\ff\ff\ff\ef\7f")  ;; Maximum finite (0x7FEFFFFFFFFFFFFF) at address 216
  (data (i32.const 224) "\00\00\00\00\00\00\10\80")  ;; Maximum negative normal (0x8010000000000000) at address 224
  (data (i32.const 232) "\ff\ff\ff\ff\ff\ff\ef\ff")  ;; Minimum finite (0xFFEFFFFFFFFFFFFF) at address 232

  ;; Basic f64.load function - loads 64-bit double from given address
  ;; Parameters: address (i32)
  ;; Returns: loaded double value (f64)
  (func (export "test_f64_load") (param $addr i32) (result f64)
    (f64.load (local.get $addr))
  )

  ;; f64.load with offset=8 - loads from address + 8
  ;; Parameters: base address (i32)
  ;; Returns: loaded double from address + 8 (f64)
  (func (export "test_f64_load_offset8") (param $addr i32) (result f64)
    (f64.load offset=8 (local.get $addr))
  )

  ;; f64.load with offset=16 - loads from address + 16
  ;; Parameters: base address (i32)
  ;; Returns: loaded double from address + 16 (f64)
  (func (export "test_f64_load_offset16") (param $addr i32) (result f64)
    (f64.load offset=16 (local.get $addr))
  )

  ;; f64.load with offset=32 - loads from address + 32
  ;; Parameters: base address (i32)
  ;; Returns: loaded double from address + 32 (f64)
  (func (export "test_f64_load_offset32") (param $addr i32) (result f64)
    (f64.load offset=32 (local.get $addr))
  )

  ;; f64.load with offset=64 - loads from address + 64
  ;; Parameters: base address (i32)
  ;; Returns: loaded double from address + 64 (f64)
  (func (export "test_f64_load_offset64") (param $addr i32) (result f64)
    (f64.load offset=64 (local.get $addr))
  )

  ;; f64.load with 8-byte alignment (natural alignment for f64)
  ;; Parameters: address (i32)
  ;; Returns: loaded double value (f64)
  (func (export "test_aligned_f64_load") (param $addr i32) (result f64)
    (f64.load align=8 (local.get $addr))
  )

  ;; f64.load with unaligned access (1-byte alignment)
  ;; Parameters: address (i32)
  ;; Returns: loaded double value (f64)
  (func (export "test_unaligned_f64_load") (param $addr i32) (result f64)
    (f64.load align=1 (i32.add (local.get $addr) (i32.const 1)))
  )

  ;; Store helper function to set up test data at runtime
  ;; Parameters: address (i32), value (f64)
  ;; Returns: none
  (func (export "store_f64_at_address") (param $addr i32) (param $value f64)
    (f64.store (local.get $addr) (local.get $value))
  )

  ;; Combined load and store test function
  ;; Parameters: source address (i32), destination address (i32)
  ;; Returns: value that was copied (f64)
  (func (export "test_f64_copy") (param $src i32) (param $dst i32) (result f64)
    (local $value f64)
    (local.set $value (f64.load (local.get $src)))
    (f64.store (local.get $dst) (local.get $value))
    (local.get $value)
  )

  ;; Memory initialization function for runtime setup
  ;; Sets up additional test values not covered by data sections
  (func (export "initialize_test_memory")
    ;; Store some runtime-calculated values
    (f64.store (i32.const 400) (f64.const 42.424242424242424))
    (f64.store (i32.const 408) (f64.const -42.424242424242424))
    (f64.store (i32.const 416) (f64.const 0.5))
    (f64.store (i32.const 424) (f64.const -0.5))
  )

  ;; Load from boundary - near end of memory (address 65528 = 65536 - 8)
  ;; Returns: double value from near memory boundary (f64)
  (func (export "test_boundary_f64_load") (param $addr i32) (result f64)
    (f64.load (i32.const 65528))
  )

  ;; Load with offset that should be within bounds
  ;; Parameters: base address (i32)
  ;; Returns: loaded double with safe offset (f64)
  (func (export "test_offset_f64_load") (param $addr i32) (result f64)
    (f64.load offset=8 (i32.const 0))
  )

  ;; Load special IEEE 754 values - positive infinity
  ;; Returns: positive infinity (f64)
  (func (export "test_special_f64_load_pos_inf") (param $addr i32) (result f64)
    (f64.load (i32.const 144))
  )

  ;; Load special IEEE 754 values - negative infinity
  ;; Returns: negative infinity (f64)
  (func (export "test_special_f64_load_neg_inf") (param $addr i32) (result f64)
    (f64.load (i32.const 152))
  )

  ;; Load special IEEE 754 values - NaN
  ;; Returns: NaN value (f64)
  (func (export "test_special_f64_load_nan") (param $addr i32) (result f64)
    (f64.load (i32.const 160))
  )

  ;; Load special IEEE 754 values - positive zero
  ;; Returns: positive zero (f64)
  (func (export "test_special_f64_load_pos_zero") (param $addr i32) (result f64)
    (f64.load (i32.const 128))
  )

  ;; Load special IEEE 754 values - negative zero
  ;; Returns: negative zero (f64)
  (func (export "test_special_f64_load_neg_zero") (param $addr i32) (result f64)
    (f64.load (i32.const 136))
  )

  ;; Out-of-bounds load test - should trap
  ;; Returns: should trap before returning (f64)
  (func (export "test_out_of_bounds_f64_load") (param $addr i32) (result f64)
    (f64.load (i32.const 65536))  ;; Beyond memory limit (65536 = 64KB, memory is 1 page = 64KB)
  )

  ;; Edge bounds load test - reading 8 bytes starting at address that would exceed memory
  ;; Returns: should trap before returning (f64)
  (func (export "test_edge_bounds_f64_load") (param $addr i32) (result f64)
    (f64.load (i32.const 65529))  ;; Address 65529 + 8 bytes = 65537 > 65536 (memory limit)
  )

  ;; Memory bounds validation function
  ;; Parameters: address (i32)
  ;; Returns: 1 if within bounds for f64.load, 0 if would cause trap
  (func (export "check_f64_bounds") (param $addr i32) (result i32)
    (if (result i32) (i32.gt_u (local.get $addr) (i32.const 65528))
      (then (i32.const 0))  ;; Out of bounds for f64 (need 8 bytes)
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