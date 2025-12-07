(module
  ;; Arithmetic and Bitwise Operations Test Module
  ;; This module tests the 10 target functions for Step 1 coverage improvement:
  ;; clz32, clz64, ctz32, ctz64, rotl32, rotr32, rotl64, rotr64, popcount32, popcount64

  ;; Count Leading Zeros (32-bit)
  (func (export "test_clz32") (param $value i32) (result i32)
    local.get $value
    i32.clz)

  ;; Count Leading Zeros (64-bit)
  (func (export "test_clz64") (param $value i64) (result i64)
    local.get $value
    i64.clz)

  ;; Count Trailing Zeros (32-bit)
  (func (export "test_ctz32") (param $value i32) (result i32)
    local.get $value
    i32.ctz)

  ;; Count Trailing Zeros (64-bit)
  (func (export "test_ctz64") (param $value i64) (result i64)
    local.get $value
    i64.ctz)

  ;; Rotate Left (32-bit)
  (func (export "test_rotl32") (param $value i32) (param $shift i32) (result i32)
    local.get $value
    local.get $shift
    i32.rotl)

  ;; Rotate Right (32-bit)
  (func (export "test_rotr32") (param $value i32) (param $shift i32) (result i32)
    local.get $value
    local.get $shift
    i32.rotr)

  ;; Rotate Left (64-bit)
  (func (export "test_rotl64") (param $value i64) (param $shift i64) (result i64)
    local.get $value
    local.get $shift
    i64.rotl)

  ;; Rotate Right (64-bit)
  (func (export "test_rotr64") (param $value i64) (param $shift i64) (result i64)
    local.get $value
    local.get $shift
    i64.rotr)

  ;; Population Count (32-bit)
  (func (export "test_popcnt32") (param $value i32) (result i32)
    local.get $value
    i32.popcnt)

  ;; Population Count (64-bit)
  (func (export "test_popcnt64") (param $value i64) (result i64)
    local.get $value
    i64.popcnt)

  ;; Edge case tests for comprehensive coverage

  ;; Test clz32 with zero input (edge case)
  (func (export "test_clz32_zero") (result i32)
    i32.const 0
    i32.clz)

  ;; Test clz64 with zero input (edge case)
  (func (export "test_clz64_zero") (result i64)
    i64.const 0
    i64.clz)

  ;; Test ctz32 with zero input (edge case)
  (func (export "test_ctz32_zero") (result i32)
    i32.const 0
    i32.ctz)

  ;; Test ctz64 with zero input (edge case)
  (func (export "test_ctz64_zero") (result i64)
    i64.const 0
    i64.ctz)

  ;; Test rotl32 with full rotation (32 bits)
  (func (export "test_rotl32_full") (param $value i32) (result i32)
    local.get $value
    i32.const 32
    i32.rotl)

  ;; Test rotr32 with full rotation (32 bits)
  (func (export "test_rotr32_full") (param $value i32) (result i32)
    local.get $value
    i32.const 32
    i32.rotr)

  ;; Test rotl64 with full rotation (64 bits)
  (func (export "test_rotl64_full") (param $value i64) (result i64)
    local.get $value
    i64.const 64
    i64.rotl)

  ;; Test rotr64 with full rotation (64 bits)
  (func (export "test_rotr64_full") (param $value i64) (result i64)
    local.get $value
    i64.const 64
    i64.rotr)

  ;; Test popcount32 with all bits set
  (func (export "test_popcount32_all_bits") (result i32)
    i32.const 0xffffffff
    i32.popcnt)

  ;; Test popcount64 with all bits set
  (func (export "test_popcount64_all_bits") (result i64)
    i64.const 0xffffffffffffffff
    i64.popcnt)
)