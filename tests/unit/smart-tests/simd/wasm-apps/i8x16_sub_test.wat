(module
  ;; i8x16.sub Test Module - Comprehensive SIMD Subtraction Validation
  ;;
  ;; This module provides test functions for validating the i8x16.sub opcode
  ;; across various scenarios: basic arithmetic, boundary values, overflow
  ;; behavior, and parallel lane processing.

  ;; Basic subtraction test: [10,10,10,...] - [3,3,3,...] = [7,7,7,...]
  (func (export "i8x16_sub_basic") (result v128)
    ;; Create minuend vector with all lanes = 10
    (v128.const i8x16 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10)
    ;; Create subtrahend vector with all lanes = 3
    (v128.const i8x16 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3)
    ;; Perform i8x16 subtraction: 10 - 3 = 7 for all lanes
    i8x16.sub
  )

  ;; Mixed signs test: [15,15,15,...] - [-10,-10,-10,...] = [25,25,25,...]
  (func (export "i8x16_sub_mixed_signs") (result v128)
    ;; Create minuend vector with all lanes = 15
    (v128.const i8x16 15 15 15 15 15 15 15 15 15 15 15 15 15 15 15 15)
    ;; Create subtrahend vector with all lanes = -10
    (v128.const i8x16 -10 -10 -10 -10 -10 -10 -10 -10 -10 -10 -10 -10 -10 -10 -10 -10)
    ;; Perform i8x16 subtraction: 15 - (-10) = 25 for all lanes
    i8x16.sub
  )

  ;; Maximum values boundary test: [127,127,127,...] - [1,1,1,...] = [126,126,126,...]
  (func (export "i8x16_sub_max_values") (result v128)
    ;; Create minuend vector with all lanes = INT8_MAX (127)
    (v128.const i8x16 127 127 127 127 127 127 127 127 127 127 127 127 127 127 127 127)
    ;; Create subtrahend vector with all lanes = 1
    (v128.const i8x16 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1)
    ;; Perform i8x16 subtraction: 127 - 1 = 126 for all lanes
    i8x16.sub
  )

  ;; Minimum values boundary test: [-128,-128,-128,...] - [-1,-1,-1,...] = [-127,-127,-127,...]
  (func (export "i8x16_sub_min_values") (result v128)
    ;; Create minuend vector with all lanes = INT8_MIN (-128)
    (v128.const i8x16 -128 -128 -128 -128 -128 -128 -128 -128 -128 -128 -128 -128 -128 -128 -128 -128)
    ;; Create subtrahend vector with all lanes = -1
    (v128.const i8x16 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1)
    ;; Perform i8x16 subtraction: -128 - (-1) = -127 for all lanes
    i8x16.sub
  )

  ;; Zero values test: [0,0,0,...] - [0,0,0,...] = [0,0,0,...]
  (func (export "i8x16_sub_zero_values") (result v128)
    ;; Create minuend vector with all lanes = 0
    (v128.const i8x16 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0)
    ;; Create subtrahend vector with all lanes = 0
    (v128.const i8x16 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0)
    ;; Perform i8x16 subtraction: 0 - 0 = 0 for all lanes
    i8x16.sub
  )

  ;; Positive overflow test: [127,127,127,...] - [-1,-1,-1,...] = [-128,-128,-128,...] (wraparound)
  (func (export "i8x16_sub_overflow") (result v128)
    ;; Create minuend vector with all lanes = INT8_MAX (127)
    (v128.const i8x16 127 127 127 127 127 127 127 127 127 127 127 127 127 127 127 127)
    ;; Create subtrahend vector with all lanes = -1
    (v128.const i8x16 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1)
    ;; Perform i8x16 subtraction: 127 - (-1) = 128 → -128 (wraparound)
    i8x16.sub
  )

  ;; Negative underflow test: [-128,-128,-128,...] - [1,1,1,...] = [127,127,127,...] (wraparound)
  (func (export "i8x16_sub_underflow") (result v128)
    ;; Create minuend vector with all lanes = INT8_MIN (-128)
    (v128.const i8x16 -128 -128 -128 -128 -128 -128 -128 -128 -128 -128 -128 -128 -128 -128 -128 -128)
    ;; Create subtrahend vector with all lanes = 1
    (v128.const i8x16 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1)
    ;; Perform i8x16 subtraction: -128 - 1 = -129 → 127 (wraparound)
    i8x16.sub
  )

  ;; Mixed lanes test: different values per lane to test parallelism
  ;; minuend: [1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16]
  ;; subtrahend: [16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1]
  ;; result: [-15,-13,-11,-9,-7,-5,-3,-1,1,3,5,7,9,11,13,15]
  (func (export "i8x16_sub_mixed_lanes") (result v128)
    ;; Create minuend vector: [1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16]
    (v128.const i8x16 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16)
    ;; Create subtrahend vector: [16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1]
    (v128.const i8x16 16 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1)
    ;; Perform i8x16 subtraction: lane-wise independent computation
    i8x16.sub
  )
)