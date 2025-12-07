(module
  ;; Function to test i64.gt_u opcode
  ;; Takes two i64 parameters and returns i32 result (1 if first > second unsigned, 0 otherwise)
  (func $i64_gt_u_test (export "i64_gt_u_test") (param $first i64) (param $second i64) (result i32)
    (local.get $first)
    (local.get $second)
    (i64.gt_u)
  )

  ;; Stack underflow test function - deliberately causes underflow
  ;; This function attempts to execute i64.gt_u without sufficient operands
  (func $stack_underflow_test (export "stack_underflow_test") (result i32)
    ;; Push only one i64 value when i64.gt_u needs two
    (i64.const 42)
    ;; This should cause stack underflow when trying to pop second operand
    ;; Uncomment next line to create underflow scenario:
    ;; (i64.gt_u)

    ;; For now, return 0 to avoid actual underflow during module compilation
    (drop)
    (i32.const 0)
  )
)