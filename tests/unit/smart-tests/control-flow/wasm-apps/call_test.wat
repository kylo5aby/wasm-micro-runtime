(module
  ;; Global variable for testing side effects
  (global $test_global (mut i32) (i32.const 0))

  ;; Basic arithmetic functions for call testing

  ;; Function: add - adds two i32 values
  (func $add (param $a i32) (param $b i32) (result i32)
    local.get $a
    local.get $b
    i32.add)

  ;; Function: multiply - multiplies two i32 values
  (func $multiply (param $a i32) (param $b i32) (result i32)
    local.get $a
    local.get $b
    i32.mul)

  ;; Function: subtract - subtracts second param from first
  (func $subtract (param $a i32) (param $b i32) (result i32)
    local.get $a
    local.get $b
    i32.sub)

  ;; Zero parameter functions

  ;; Function: get_constant - returns constant value with no parameters
  (func $get_constant (result i32)
    i32.const 42)

  ;; Function: get_pi_approximation - returns approximation of pi * 100
  (func $get_pi_approximation (result i32)
    i32.const 314)

  ;; Function: get_timestamp - returns mock timestamp
  (func $get_timestamp (result i32)
    i32.const 1638360000)

  ;; Single parameter functions

  ;; Function: square - squares an i32 value
  (func $square (param $x i32) (result i32)
    local.get $x
    local.get $x
    i32.mul)

  ;; Identity functions for edge case testing

  ;; Function: identity_i32 - returns input unchanged
  (func $identity_i32 (param $x i32) (result i32)
    local.get $x)

  ;; Mixed type parameter function

  ;; Function: add_i32_f64 - adds i32 and f64, returns i32
  (func $add_i32_f64 (param $i i32) (param $f f64) (result i32)
    local.get $i
    local.get $f
    i32.trunc_f64_s
    i32.add)

  ;; Multiple return value functions

  ;; Function: divmod - returns quotient and remainder
  (func $divmod (param $a i32) (param $b i32) (result i32 i32)
    local.get $a
    local.get $b
    i32.div_s
    local.get $a
    local.get $b
    i32.rem_s)

  ;; Function: min_max - returns minimum and maximum of two values
  (func $min_max (param $a i32) (param $b i32) (result i32 i32)
    ;; Return min first
    local.get $a
    local.get $b
    local.get $a
    local.get $b
    i32.lt_s
    select
    ;; Return max second
    local.get $a
    local.get $b
    local.get $a
    local.get $b
    i32.gt_s
    select)

  ;; Void functions (no return values)

  ;; Function: set_global - sets global variable value
  (func $set_global (param $value i32)
    local.get $value
    global.set $test_global)

  ;; Function: get_global - gets global variable value
  (func $get_global (result i32)
    global.get $test_global)

  ;; Function: noop - does nothing
  (func $noop)

  ;; Boundary testing functions

  ;; Function: first_function - represents first function in module
  (func $first_function (result i32)
    i32.const 1)

  ;; Function: last_function - represents last function in module
  (func $last_function (result i32)
    i32.const 999)

  ;; High parameter count function

  ;; Function: sum_eight - sums eight i32 parameters
  (func $sum_eight (param $p1 i32) (param $p2 i32) (param $p3 i32) (param $p4 i32)
                   (param $p5 i32) (param $p6 i32) (param $p7 i32) (param $p8 i32)
                   (result i32)
    local.get $p1
    local.get $p2
    i32.add
    local.get $p3
    i32.add
    local.get $p4
    i32.add
    local.get $p5
    i32.add
    local.get $p6
    i32.add
    local.get $p7
    i32.add
    local.get $p8
    i32.add)

  ;; Recursive functions

  ;; Function: factorial - recursive factorial calculation
  (func $factorial (param $n i32) (result i32)
    local.get $n
    i32.const 1
    i32.le_s
    if (result i32)
      i32.const 1
    else
      local.get $n
      local.get $n
      i32.const 1
      i32.sub
      call $factorial
      i32.mul
    end)

  ;; Function: fibonacci - recursive fibonacci calculation
  (func $fibonacci (param $n i32) (result i32)
    local.get $n
    i32.const 2
    i32.lt_s
    if (result i32)
      local.get $n
    else
      local.get $n
      i32.const 1
      i32.sub
      call $fibonacci
      local.get $n
      i32.const 2
      i32.sub
      call $fibonacci
      i32.add
    end)

  ;; Extreme value functions

  ;; Function: get_i32_max - returns INT32_MAX
  (func $get_i32_max (result i32)
    i32.const 0x7FFFFFFF)

  ;; Function: get_i32_min - returns INT32_MIN
  (func $get_i32_min (result i32)
    i32.const 0x80000000)

  ;; Function: get_zero - returns zero
  (func $get_zero (result i32)
    i32.const 0)

  ;; Export all functions for testing
  (export "add" (func $add))
  (export "multiply" (func $multiply))
  (export "subtract" (func $subtract))
  (export "get_constant" (func $get_constant))
  (export "get_pi_approximation" (func $get_pi_approximation))
  (export "get_timestamp" (func $get_timestamp))
  (export "square" (func $square))
  (export "identity_i32" (func $identity_i32))
  (export "add_i32_f64" (func $add_i32_f64))
  (export "divmod" (func $divmod))
  (export "min_max" (func $min_max))
  (export "set_global" (func $set_global))
  (export "get_global" (func $get_global))
  (export "noop" (func $noop))
  (export "first_function" (func $first_function))
  (export "last_function" (func $last_function))
  (export "sum_eight" (func $sum_eight))
  (export "factorial" (func $factorial))
  (export "fibonacci" (func $fibonacci))
  (export "get_i32_max" (func $get_i32_max))
  (export "get_i32_min" (func $get_i32_min))
  (export "get_zero" (func $get_zero))
)