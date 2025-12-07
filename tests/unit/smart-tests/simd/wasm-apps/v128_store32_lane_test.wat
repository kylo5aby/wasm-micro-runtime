(module
  ;; Memory for testing v128.store32_lane operations (1 page = 64KB)
  (memory (export "memory") 1)

  ;; Test data constants for distinct lane values
  (data (i32.const 0x10000) "\11\11\11\11\22\22\22\22\33\33\33\33\44\44\44\44")

  ;; Helper function to create test v128 with distinct lane values
  ;; Returns v128 with lanes: [0x11111111, 0x22222222, 0x33333333, 0x44444444]
  (func $create_test_vector (result v128)
    ;; Load the 16-byte test data starting at offset 0x10000
    (v128.load (i32.const 0x10000))
  )

  ;; Test function: Store lane 0 to memory
  ;; Parameters: base_addr (i32)
  ;; Operation: v128.store32_lane with lane index 0
  (func (export "test_store_lane0") (param $base_addr i32)
    ;; Store lane 0 (0x11111111) to memory at base_addr
    (local.get $base_addr)
    (call $create_test_vector)
    (v128.store32_lane 0)
  )

  ;; Test function: Store lane 1 to memory
  ;; Parameters: base_addr (i32)
  ;; Operation: v128.store32_lane with lane index 1
  (func (export "test_store_lane1") (param $base_addr i32)
    (local.get $base_addr)
    (call $create_test_vector)
    (v128.store32_lane 1)
  )

  ;; Test function: Store lane 2 to memory
  ;; Parameters: base_addr (i32)
  ;; Operation: v128.store32_lane with lane index 2
  (func (export "test_store_lane2") (param $base_addr i32)
    (local.get $base_addr)
    (call $create_test_vector)
    (v128.store32_lane 2)
  )

  ;; Test function: Store lane 3 to memory
  ;; Parameters: base_addr (i32)
  ;; Operation: v128.store32_lane with lane index 3
  (func (export "test_store_lane3") (param $base_addr i32)
    (local.get $base_addr)
    (call $create_test_vector)
    (v128.store32_lane 3)
  )

  ;; Test function: Store at valid boundary address (65532)
  ;; Parameters: base_addr (i32) - should be 65532 (64KB - 4 bytes)
  ;; Operation: Store 0xBBBBBBBB at boundary
  (func (export "test_store_boundary_valid") (param $base_addr i32)
    (local.get $base_addr)
    ;; Create vector with 0xBBBBBBBB in lane 0
    (v128.const i32x4 0xBBBBBBBB 0x00000000 0x00000000 0x00000000)
    (v128.store32_lane 0)
  )

  ;; Test function: Store at invalid boundary address (should trap)
  ;; Parameters: base_addr (i32) - should be 65536 or higher (beyond memory)
  ;; Operation: Attempt store beyond memory limit
  (func (export "test_store_boundary_invalid") (param $base_addr i32)
    (local.get $base_addr)
    (v128.const i32x4 0xDEADBEEF 0x00000000 0x00000000 0x00000000)
    ;; This should trap due to out-of-bounds access
    (v128.store32_lane 0)
  )

  ;; Test function: Store at partial boundary address (should trap)
  ;; Parameters: base_addr (i32) - should be 65534 (only 2 bytes available, need 4)
  ;; Operation: Attempt partial store at boundary
  (func (export "test_store_boundary_partial") (param $base_addr i32)
    (local.get $base_addr)
    (v128.const i32x4 0xDEADBEEF 0x00000000 0x00000000 0x00000000)
    ;; This should trap due to partial out-of-bounds access
    (v128.store32_lane 0)
  )

  ;; Test function: Store with zero memarg offset
  ;; Parameters: base_addr (i32)
  ;; Operation: Direct address store (offset = 0 implicit)
  (func (export "test_store_zero_offset") (param $base_addr i32)
    (local.get $base_addr)
    (v128.const i32x4 0xCCCCCCCC 0x00000000 0x00000000 0x00000000)
    ;; Store with implicit memarg offset = 0
    (v128.store32_lane 0)
  )

  ;; Test function: Store with small memarg offset
  ;; Parameters: base_addr (i32)
  ;; Operation: Store at base_addr + 16
  (func (export "test_store_small_offset") (param $base_addr i32)
    ;; Add offset to base address manually since memarg offset is compile-time
    (local.get $base_addr)
    (i32.const 16)
    (i32.add)
    (v128.const i32x4 0xDDDDDDDD 0x00000000 0x00000000 0x00000000)
    ;; Store at computed address
    (v128.store32_lane 0)
  )

  ;; Test function: Store with large memarg offset
  ;; Parameters: base_addr (i32)
  ;; Operation: Store at base_addr + 0x7000
  (func (export "test_store_large_offset") (param $base_addr i32)
    ;; Add large offset to base address
    (local.get $base_addr)
    (i32.const 0x7000)
    (i32.add)
    (v128.const i32x4 0xEEEEEEEE 0x00000000 0x00000000 0x00000000)
    ;; Store at computed address
    (v128.store32_lane 0)
  )

  ;; Test function: Store zero value (0x00000000)
  ;; Parameters: base_addr (i32)
  ;; Operation: Store zero pattern value
  (func (export "test_store_zero_value") (param $base_addr i32)
    (local.get $base_addr)
    (v128.const i32x4 0x00000000 0x00000000 0x00000000 0x00000000)
    (v128.store32_lane 0)
  )

  ;; Test function: Store maximum value (0xFFFFFFFF)
  ;; Parameters: base_addr (i32)
  ;; Operation: Store maximum unsigned 32-bit value
  (func (export "test_store_max_value") (param $base_addr i32)
    (local.get $base_addr)
    (v128.const i32x4 0xFFFFFFFF 0x00000000 0x00000000 0x00000000)
    (v128.store32_lane 0)
  )

  ;; Test function: Store minimum signed value (0x80000000)
  ;; Parameters: base_addr (i32)
  ;; Operation: Store minimum signed 32-bit value (-2147483648)
  (func (export "test_store_min_value") (param $base_addr i32)
    (local.get $base_addr)
    (v128.const i32x4 0x80000000 0x00000000 0x00000000 0x00000000)
    (v128.store32_lane 0)
  )

  ;; Test function: Store maximum signed value (0x7FFFFFFF)
  ;; Parameters: base_addr (i32)
  ;; Operation: Store maximum signed 32-bit value (2147483647)
  (func (export "test_store_max_signed_value") (param $base_addr i32)
    (local.get $base_addr)
    (v128.const i32x4 0x7FFFFFFF 0x00000000 0x00000000 0x00000000)
    (v128.store32_lane 0)
  )

  ;; Test function: Store pattern 1 value (0xAAAAAAAA)
  ;; Parameters: base_addr (i32)
  ;; Operation: Store checkerboard pattern 1
  (func (export "test_store_pattern1") (param $base_addr i32)
    (local.get $base_addr)
    (v128.const i32x4 0xAAAAAAAA 0x00000000 0x00000000 0x00000000)
    (v128.store32_lane 0)
  )

  ;; Test function: Store pattern 2 value (0x55555555)
  ;; Parameters: base_addr (i32)
  ;; Operation: Store checkerboard pattern 2
  (func (export "test_store_pattern2") (param $base_addr i32)
    (local.get $base_addr)
    (v128.const i32x4 0x55555555 0x00000000 0x00000000 0x00000000)
    (v128.store32_lane 0)
  )
)