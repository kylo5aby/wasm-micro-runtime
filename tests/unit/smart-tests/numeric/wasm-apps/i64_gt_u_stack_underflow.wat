(module
  ;; Intentional stack underflow test - this module will fail validation
  ;; This demonstrates what happens when i64.gt_u doesn't have enough operands
  (func $underflow_test (export "underflow_test") (result i32)
    ;; Push only one i64 value but i64.gt_u needs two
    (i64.const 42)
    ;; This will cause stack underflow - WASM validation should catch this
    (i64.gt_u)
    ;; Convert i64 result to i32 (this line won't be reached due to validation error)
  )
)