(module
  ;; Import memory for SIMD operations
  (memory $mem 1)

  ;; Test function: Basic pairwise addition with positive values
  ;; Input: i8x16 [1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16]
  ;; Expected output: i16x8 [3,7,11,15,19,23,27,31]
  (func $test_basic_pairwise_addition (param $v1 v128) (result v128)
    local.get $v1
    i16x8.extadd_pairwise_i8x16_s
  )

  ;; Test function: Negative value handling with sign extension
  ;; Input: i8x16 [-1,-2,-3,-4,-5,-6,-7,-8,-9,-10,-11,-12,-13,-14,-15,-16]
  ;; Expected output: i16x8 [-3,-7,-11,-15,-19,-23,-27,-31]
  (func $test_negative_values (param $v1 v128) (result v128)
    local.get $v1
    i16x8.extadd_pairwise_i8x16_s
  )

  ;; Test function: Mixed sign values that cancel out
  ;; Input: i8x16 [1,-1,2,-2,3,-3,4,-4,5,-5,6,-6,7,-7,8,-8]
  ;; Expected output: i16x8 [0,0,0,0,0,0,0,0]
  (func $test_mixed_signs (param $v1 v128) (result v128)
    local.get $v1
    i16x8.extadd_pairwise_i8x16_s
  )

  ;; Test function: Edge value combinations with boundary conditions
  ;; Input: i8x16 [127,-128,0,0,127,127,-128,-128,1,-1,126,-127,125,-126,124,-125]
  ;; Expected output: i16x8 [-1,0,254,-256,0,-1,-1,-1]
  (func $test_edge_values (param $v1 v128) (result v128)
    local.get $v1
    i16x8.extadd_pairwise_i8x16_s
  )

  ;; Export all test functions for C++ integration
  (export "test_basic_pairwise_addition" (func $test_basic_pairwise_addition))
  (export "test_negative_values" (func $test_negative_values))
  (export "test_mixed_signs" (func $test_mixed_signs))
  (export "test_edge_values" (func $test_edge_values))
)