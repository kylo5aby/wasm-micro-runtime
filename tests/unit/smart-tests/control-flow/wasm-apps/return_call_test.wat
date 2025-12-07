(module
  ;; Function type definitions for various tail call scenarios
  (type $i32_to_i32 (func (param i32) (result i32)))
  (type $two_i32_to_i32 (func (param i32 i32) (result i32)))
  (type $void_to_i32 (func (result i32)))
  (type $void_to_void (func))

  ;; Helper functions called by return_call instructions

  ;; Double the input value
  (func $double (type $i32_to_i32) (param $x i32) (result i32)
    (i32.mul (local.get $x) (i32.const 2))
  )

  ;; Triple the input value
  (func $triple (type $i32_to_i32) (param $x i32) (result i32)
    (i32.mul (local.get $x) (i32.const 3))
  )

  ;; Add two integers
  (func $add (type $two_i32_to_i32) (param $a i32) (param $b i32) (result i32)
    (i32.add (local.get $a) (local.get $b))
  )

  ;; Return constant value (parameterless function)
  (func $get_constant (type $void_to_i32) (result i32)
    (i32.const 42)
  )

  ;; Identity function (returns input unchanged)
  (func $identity (type $i32_to_i32) (param $x i32) (result i32)
    (local.get $x)
  )

  ;; Factorial helper for tail recursion
  (func $factorial_helper (type $two_i32_to_i32) (param $n i32) (param $acc i32) (result i32)
    (if (result i32) (i32.le_s (local.get $n) (i32.const 1))
      (then (local.get $acc))
      (else
        (return_call $factorial_helper
          (i32.sub (local.get $n) (i32.const 1))
          (i32.mul (local.get $n) (local.get $acc))
        )
      )
    )
  )

  ;; Even/odd mutual recursion helpers
  (func $is_even_helper (type $i32_to_i32) (param $n i32) (result i32)
    (if (result i32) (i32.eq (local.get $n) (i32.const 0))
      (then (i32.const 1))  ;; 0 is even
      (else (return_call $is_odd_helper (i32.sub (local.get $n) (i32.const 1))))
    )
  )

  (func $is_odd_helper (type $i32_to_i32) (param $n i32) (result i32)
    (if (result i32) (i32.eq (local.get $n) (i32.const 0))
      (then (i32.const 0))  ;; 0 is not odd
      (else (return_call $is_even_helper (i32.sub (local.get $n) (i32.const 1))))
    )
  )

  ;; Countdown recursive function
  (func $countdown (type $i32_to_i32) (param $n i32) (result i32)
    (if (result i32) (i32.le_s (local.get $n) (i32.const 0))
      (then (i32.const 0))  ;; Base case: return 0
      (else (return_call $countdown (i32.sub (local.get $n) (i32.const 1))))
    )
  )

  ;; Test functions exported for C++ test code

  ;; Basic tail call test: calls double function via return_call
  (func $test_basic_tail_call (export "test_basic_tail_call") (type $i32_to_i32)
    (param $x i32) (result i32)
    ;; Use return_call to call double function
    (return_call $double (local.get $x))
  )

  ;; Multi-parameter tail call test: calls add function via return_call
  (func $test_multi_param_tail_call (export "test_multi_param_tail_call")
    (type $two_i32_to_i32) (param $a i32) (param $b i32) (result i32)
    ;; Use return_call to call add function
    (return_call $add (local.get $a) (local.get $b))
  )

  ;; Zero-parameter tail call test: calls constant function via return_call
  (func $test_zero_param_tail_call (export "test_zero_param_tail_call")
    (type $void_to_i32) (result i32)
    ;; Use return_call to call parameterless function
    (return_call $get_constant)
  )

  ;; First function boundary test: calls function 0 (double)
  (func $test_first_function_tail_call (export "test_first_function_tail_call")
    (type $i32_to_i32) (param $x i32) (result i32)
    ;; return_call to first function in module (index 0 = $double)
    (return_call $double (local.get $x))
  )

  ;; Last function boundary test: calls last helper function
  (func $test_last_function_tail_call (export "test_last_function_tail_call")
    (type $i32_to_i32) (param $x i32) (result i32)
    ;; return_call to triple function
    (return_call $triple (local.get $x))
  )

  ;; Deep recursion test: countdown from large number
  (func $test_recursive_countdown (export "test_recursive_countdown")
    (type $i32_to_i32) (param $n i32) (result i32)
    ;; Use return_call for recursive countdown
    (return_call $countdown (local.get $n))
  )

  ;; Tail recursive factorial test
  (func $test_tail_factorial (export "test_tail_factorial")
    (type $i32_to_i32) (param $n i32) (result i32)
    ;; Start factorial computation with accumulator = 1
    (return_call $factorial_helper (local.get $n) (i32.const 1))
  )

  ;; Mutual recursion test for even numbers
  (func $test_mutual_recursion_even (export "test_mutual_recursion_even")
    (type $i32_to_i32) (param $n i32) (result i32)
    ;; Test if number is even using mutual recursion
    (return_call $is_even_helper (local.get $n))
  )

  ;; Identity function tail call test
  (func $test_identity_tail_call (export "test_identity_tail_call")
    (type $i32_to_i32) (param $x i32) (result i32)
    ;; Use return_call to call identity function
    (return_call $identity (local.get $x))
  )
)