;; Enhanced WASM module for f64x2.sub opcode testing
;; Tests element-wise subtraction of two f64x2 SIMD vectors
;; Provides comprehensive test functions for various f64x2.sub scenarios

(module
  ;; Basic f64x2.sub function for standard subtraction operations
  ;; Performs element-wise subtraction: result[i] = minuend[i] - subtrahend[i]
  (func $f64x2_sub_basic (export "f64x2_sub_basic") (param $minuend v128) (param $subtrahend v128) (result v128)
    local.get $minuend
    local.get $subtrahend
    f64x2.sub
  )

  ;; f64x2.sub function for zero value testing
  ;; Tests IEEE 754 zero arithmetic: x - (+0.0), x - (-0.0), (+0.0) - (-0.0)
  (func $f64x2_sub_zero (export "f64x2_sub_zero") (param $minuend v128) (param $subtrahend v128) (result v128)
    local.get $minuend
    local.get $subtrahend
    f64x2.sub
  )

  ;; f64x2.sub function for infinity value testing
  ;; Tests infinity arithmetic: inf - inf, inf - (-inf), finite - inf
  (func $f64x2_sub_infinity (export "f64x2_sub_infinity") (param $minuend v128) (param $subtrahend v128) (result v128)
    local.get $minuend
    local.get $subtrahend
    f64x2.sub
  )

  ;; f64x2.sub function for NaN propagation testing
  ;; Tests NaN arithmetic: NaN - x, x - NaN, NaN - NaN
  (func $f64x2_sub_nan (export "f64x2_sub_nan") (param $minuend v128) (param $subtrahend v128) (result v128)
    local.get $minuend
    local.get $subtrahend
    f64x2.sub
  )

  ;; f64x2.sub function for boundary value testing
  ;; Tests extreme values: DBL_MAX operations, overflow/underflow handling
  (func $f64x2_sub_boundary (export "f64x2_sub_boundary") (param $minuend v128) (param $subtrahend v128) (result v128)
    local.get $minuend
    local.get $subtrahend
    f64x2.sub
  )

  ;; Helper function to create f64x2 vector from two f64 values
  ;; Useful for test setup and vector construction
  (func $make_f64x2 (export "make_f64x2") (param $a f64) (param $b f64) (result v128)
    local.get $a
    f64x2.splat
    local.get $b
    f64x2.replace_lane 1
  )

  ;; Helper function to extract first lane from f64x2 vector
  ;; Enables individual element validation in tests
  (func $extract_lane_0 (export "extract_lane_0") (param $v v128) (result f64)
    local.get $v
    f64x2.extract_lane 0
  )

  ;; Helper function to extract second lane from f64x2 vector
  ;; Enables individual element validation in tests
  (func $extract_lane_1 (export "extract_lane_1") (param $v v128) (result f64)
    local.get $v
    f64x2.extract_lane 1
  )

  ;; Comprehensive test function combining multiple f64x2.sub operations
  ;; Tests various scenarios in a single function call for performance validation
  (func $f64x2_sub_comprehensive (export "f64x2_sub_comprehensive")
        (param $v1 v128) (param $v2 v128) (param $v3 v128) (param $v4 v128)
        (result v128)
    ;; Perform multiple subtraction operations
    local.get $v1
    local.get $v2
    f64x2.sub

    local.get $v3
    local.get $v4
    f64x2.sub

    ;; Subtract the results to test chained operations
    f64x2.sub
  )

  ;; Performance test function for repeated f64x2.sub operations
  ;; Validates consistent behavior under repeated execution
  (func $f64x2_sub_performance (export "f64x2_sub_performance")
        (param $input1 v128) (param $input2 v128) (result v128)
    (local $result v128)
    (local $i i32)

    local.get $input1
    local.set $result

    ;; Perform 100 subtraction operations for performance testing
    i32.const 0
    local.set $i

    loop $performance_loop
      local.get $result
      local.get $input2
      f64x2.sub
      local.set $result

      local.get $i
      i32.const 1
      i32.add
      local.tee $i
      i32.const 100
      i32.lt_s
      br_if $performance_loop
    end

    local.get $result
  )

  ;; Memory operations test function
  ;; Tests f64x2.sub with memory-loaded vectors for realistic usage scenarios
  (memory $mem 1)

  (func $f64x2_sub_memory (export "f64x2_sub_memory") (param $offset1 i32) (param $offset2 i32) (result v128)
    ;; Load two f64x2 vectors from memory and perform subtraction
    local.get $offset1
    v128.load

    local.get $offset2
    v128.load

    f64x2.sub
  )

  ;; Initialize memory with test data for memory operations
  (func $init_memory (export "init_memory")
    ;; Store test vectors at specific memory locations
    i32.const 0
    f64.const 1.5
    f64.store

    i32.const 8
    f64.const 2.5
    f64.store

    i32.const 16
    f64.const 0.5
    f64.store

    i32.const 24
    f64.const 1.0
    f64.store
  )
)