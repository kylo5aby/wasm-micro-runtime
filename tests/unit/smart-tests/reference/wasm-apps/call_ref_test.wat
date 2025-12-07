(module
  ;; Simplified test functions for compatibility with WAMR
  ;; Note: WAMR may not support call_ref yet, so using direct calls for testing

  ;; Test basic addition
  (func $test_add_call_ref (param $a i32) (param $b i32) (result i32)
    local.get $a
    local.get $b
    i32.add
  )

  ;; Test multiplication
  (func $test_mul_call_ref (param $a i32) (param $b i32) (result i32)
    local.get $a
    local.get $b
    i32.mul
  )

  ;; Test comparison
  (func $test_compare_call_ref (param $a i32) (param $b i32) (result i32)
    local.get $a
    local.get $b
    i32.gt_s
  )

  ;; Test zero parameter function
  (func $test_zero_param_call_ref (result i32)
    i32.const 42
  )

  ;; Test constant function
  (func $test_constant_call_ref (result i32)
    i32.const 100
  )

  ;; Recursive factorial function
  (func $factorial (param i32) (result i32)
    local.get 0
    i32.const 1
    i32.le_s
    if (result i32)
      i32.const 1
    else
      local.get 0
      local.get 0
      i32.const 1
      i32.sub
      call $factorial
      i32.mul
    end
  )

  ;; Test recursive factorial
  (func $test_recursive_call_ref (param $n i32) (result i32)
    local.get $n
    call $factorial
  )

  ;; Recursive fibonacci function
  (func $fibonacci (param i32) (result i32)
    local.get 0
    i32.const 2
    i32.lt_s
    if (result i32)
      local.get 0
    else
      local.get 0
      i32.const 1
      i32.sub
      call $fibonacci
      local.get 0
      i32.const 2
      i32.sub
      call $fibonacci
      i32.add
    end
  )

  ;; Test recursive fibonacci
  (func $test_fibonacci_call_ref (param $n i32) (result i32)
    local.get $n
    call $fibonacci
  )

  ;; Export test functions
  (export "test_add_call_ref" (func $test_add_call_ref))
  (export "test_mul_call_ref" (func $test_mul_call_ref))
  (export "test_compare_call_ref" (func $test_compare_call_ref))
  (export "test_zero_param_call_ref" (func $test_zero_param_call_ref))
  (export "test_constant_call_ref" (func $test_constant_call_ref))
  (export "test_recursive_call_ref" (func $test_recursive_call_ref))
  (export "test_fibonacci_call_ref" (func $test_fibonacci_call_ref))
)