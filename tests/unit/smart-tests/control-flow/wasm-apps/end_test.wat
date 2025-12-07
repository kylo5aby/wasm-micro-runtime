(module
  ;; Basic block termination tests
  (func $test_empty_block (export "test_empty_block") (result i32)
    (block
      ;; Empty block - immediate end
    )
    i32.const 42
  )

  (func $test_single_result_block (export "test_single_result_block") (result i32)
    (block (result i32)
      i32.const 100
    )
  )

  (func $test_param_result_block (export "test_param_result_block") (param i32) (result i32)
    local.get 0
    (block (param i32) (result i32)
      ;; Pass through parameter value
    )
  )

  ;; Loop structure termination tests
  (func $test_simple_loop (export "test_simple_loop") (result i32)
    (loop (result i32)
      i32.const 1
      ;; Loop ends here, executes once
    )
  )

  (func $test_loop_with_break (export "test_loop_with_break") (result i32)
    (loop (result i32)
      i32.const 0
      br 0  ;; Break immediately to loop start - effectively exits
    )
  )

  (func $test_counting_loop (export "test_counting_loop") (param i32) (result i32)
    (loop (result i32)
      local.get 0
      ;; Simple counting loop that returns the input value
    )
  )

  ;; Conditional structure termination tests
  (func $test_if_then (export "test_if_then") (param i32) (result i32)
    local.get 0
    (if (result i32)
      (then
        i32.const 10
      )
      (else
        i32.const 0
      )
    )
  )

  (func $test_if_then_else (export "test_if_then_else") (param i32) (result i32)
    local.get 0
    (if (result i32)
      (then
        i32.const 20
      )
      (else
        i32.const 30
      )
    )
  )

  ;; Nested structure tests
  (func $test_nested_blocks (export "test_nested_blocks") (result i32)
    (block (result i32)
      (block (result i32)
        (block (result i32)
          i32.const 123
        )
      )
    )
  )

  (func $test_mixed_nesting (export "test_mixed_nesting") (param i32) (result i32)
    (block (result i32)
      local.get 0
      (loop (param i32) (result i32)
        local.get 0
        (if (param i32) (result i32)
          (then
            i32.const 3
            i32.mul
          )
          (else
            i32.const 1
            i32.add
          )
        )
      )
    )
  )

  (func $test_deep_nesting (export "test_deep_nesting") (param i32) (result i32)
    (block (result i32)
      local.get 0
      (block (param i32) (result i32)
        (block (param i32) (result i32)
          (block (param i32) (result i32)
            i32.const 2
            i32.mul
            i32.const 2
            i32.mul
          )
        )
      )
    )
  )

  ;; Empty structure tests
  (func $test_empty_loop (export "test_empty_loop") (result i32)
    (loop
      ;; Empty loop body
    )
    i32.const 0
  )

  (func $test_empty_conditional (export "test_empty_conditional") (param i32) (result i32)
    local.get 0
    (if
      (then
        ;; Empty then branch
      )
    )
    i32.const 0
  )

  ;; Unreachable code path tests
  (func $test_unreachable_block (export "test_unreachable_block") (result i32)
    (block (result i32)
      i32.const 99
      return
      unreachable  ;; This should never be reached
    )
  )

  (func $test_early_return (export "test_early_return") (param i32) (result i32)
    local.get 0
    return
    (block
      unreachable  ;; This block should never be reached
    )
    i32.const 0
  )

  ;; Type validation tests
  (func $test_correct_types (export "test_correct_types") (result i32)
    (block (result i32)
      i32.const 777
    )
  )

  (func $test_multi_result_types (export "test_multi_result_types") (param i32) (result i32)
    (block (result i32)
      local.get 0
      (block (param i32) (result i32)
        ;; Pass through value maintaining type consistency
      )
    )
  )
)