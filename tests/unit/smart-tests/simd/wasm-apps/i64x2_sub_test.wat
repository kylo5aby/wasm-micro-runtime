;; Enhanced WASM module for i64x2.sub opcode testing
;; Tests element-wise subtraction of two i64x2 SIMD vectors
;; Provides comprehensive test functions for various i64x2.sub scenarios

(module
  ;; Basic i64x2.sub function for standard subtraction operations
  ;; Performs element-wise subtraction: result[i] = minuend[i] - subtrahend[i]
  (func $i64x2_sub_test (export "i64x2_sub_test") (param $minuend v128) (param $subtrahend v128) (result v128)
    local.get $minuend
    local.get $subtrahend
    i64x2.sub
  )

  ;; Helper function to create i64x2 vector from two i64 values
  ;; Useful for test setup and vector construction
  (func $make_i64x2 (export "make_i64x2") (param $a i64) (param $b i64) (result v128)
    local.get $a
    i64x2.splat
    local.get $b
    i64x2.replace_lane 1
  )

  ;; Helper function to extract first lane from i64x2 vector
  ;; Enables individual element validation in tests
  (func $extract_lane_0 (export "extract_lane_0") (param $v v128) (result i64)
    local.get $v
    i64x2.extract_lane 0
  )

  ;; Helper function to extract second lane from i64x2 vector
  ;; Enables individual element validation in tests
  (func $extract_lane_1 (export "extract_lane_1") (param $v v128) (result i64)
    local.get $v
    i64x2.extract_lane 1
  )
)