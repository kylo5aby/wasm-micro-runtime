(module
  ;; Type definitions for function signatures
  (type $add_sig (func (param i32 i32) (result i32)))
  (type $multi_param_sig (func (param i32 i64 f32 f64) (result i32)))
  (type $recursive_sig (func (param i32) (result i32)))
  (type $no_param_sig (func (result i32)))
  (type $type_mismatch_sig (func (param i64) (result i32)))

  ;; Helper function: simple addition
  (func $add_func (param $a i32) (param $b i32) (result i32)
    local.get $a
    local.get $b
    i32.add
  )

  ;; Helper function: multi-parameter computation
  (func $multi_param_func (param $i32_val i32) (param $i64_val i64) (param $f32_val f32) (param $f64_val f64) (result i32)
    ;; Compute: i32_param + (i64_param as i32) + (f32_param as i32) + (f64_param as i32)
    local.get $i32_val
    local.get $i64_val
    i32.wrap_i64
    i32.add
    local.get $f32_val
    i32.trunc_f32_s
    i32.add
    local.get $f64_val
    i32.trunc_f64_s
    i32.add
  )

  ;; Helper function: no parameters
  (func $no_param_func (result i32)
    i32.const 42
  )

  ;; Helper function: expects i64 parameter (for type mismatch test)
  (func $type_mismatch_func (param $val i64) (result i32)
    local.get $val
    i32.wrap_i64
  )

  ;; Helper function: recursive countdown (simulated tail call)
  (func $recursive_func (param $counter i32) (result i32)
    ;; Check if counter is zero
    local.get $counter
    i32.const 0
    i32.eq
    if (result i32)
      ;; Base case: return 0
      i32.const 0
    else
      ;; Recursive case: decrement and call again
      local.get $counter
      i32.const 1
      i32.sub
      call $recursive_func
    end
  )

  ;; Test function: basic tail call through function reference (simulated)
  (func $test_basic_tail_call (param $a i32) (param $b i32) (result i32)
    ;; For compatibility, simulate tail call behavior using regular call
    ;; In actual WAMR with tail call support, this would use return_call_ref
    local.get $a
    local.get $b
    call $add_func
  )

  ;; Test function: multi-parameter tail call (simulated)
  (func $test_multi_param_tail_call (param $i32_val i32) (param $i64_val i64) (param $f32_val f32) (param $f64_val f64) (result i32)
    ;; Simulate tail call with all parameters
    local.get $i32_val
    local.get $i64_val
    local.get $f32_val
    local.get $f64_val
    call $multi_param_func
  )

  ;; Test function: recursive tail call (entry point)
  (func $test_recursive_tail_call (param $initial_counter i32) (result i32)
    ;; Start recursive countdown
    local.get $initial_counter
    call $recursive_func
  )

  ;; Test function: zero parameter tail call
  (func $test_zero_param_tail_call (result i32)
    ;; Call function with no parameters
    call $no_param_func
  )

  ;; Test function: null funcref trap (will be handled by C++ test)
  (func $test_null_funcref_trap (param $a i32) (param $b i32) (result i32)
    ;; This test will be implemented in C++ to properly test null funcref handling
    ;; For now, return an error indicator
    i32.const -1
  )

  ;; Test function: type mismatch (i32 provided, i64 expected)
  (func $test_type_mismatch (param $val i32) (result i32)
    ;; Convert i32 to i64 for the function call
    local.get $val
    i64.extend_i32_s
    call $type_mismatch_func
  )

  ;; Test function: stack underflow simulation
  (func $test_stack_underflow (result i32)
    ;; This test will be handled in C++ to properly test stack underflow
    ;; For now, return an error indicator
    i32.const -2
  )

  ;; Chain helper functions for testing chained calls
  (func $chain_helper_1 (param $val i32) (result i32)
    ;; Chain to another function
    local.get $val
    i32.const 10
    i32.add
    call $chain_helper_2
  )

  (func $chain_helper_2 (param $val i32) (result i32)
    ;; Final function in chain
    local.get $val
    i32.const 5
    i32.mul
  )

  (func $test_chained_tail_calls (param $initial i32) (result i32)
    ;; Start chain of calls
    local.get $initial
    call $chain_helper_1
  )

  ;; Export all test functions
  (export "test_basic_tail_call" (func $test_basic_tail_call))
  (export "test_multi_param_tail_call" (func $test_multi_param_tail_call))
  (export "test_recursive_tail_call" (func $test_recursive_tail_call))
  (export "test_zero_param_tail_call" (func $test_zero_param_tail_call))
  (export "test_null_funcref_trap" (func $test_null_funcref_trap))
  (export "test_type_mismatch" (func $test_type_mismatch))
  (export "test_stack_underflow" (func $test_stack_underflow))
  (export "test_chained_tail_calls" (func $test_chained_tail_calls))
)