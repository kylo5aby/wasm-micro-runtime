(module
  ;; Type definitions for function signatures
  (type $t0 (func (param i32 i32) (result i32)))     ;; Binary operation: (i32, i32) -> i32
  (type $t1 (func (param i32 i32 i32) (result i32))) ;; Triple operation: (i32, i32, i32) -> i32
  (type $t2 (func (result i32)))                     ;; Constant function: () -> i32
  (type $t3 (func (param i32) (result i32)))         ;; Unary operation: i32 -> i32

  ;; Function table with 10 entries (indices 0-9)
  (table funcref (elem $add_func $mul_func $triple_func $sub_func $const42_func $const100_func $recursive_countdown $recursive_factorial $fibonacci $gcd))

  ;; Addition function: adds two i32 values
  (func $add_func (type $t0)
    local.get 0
    local.get 1
    i32.add
  )

  ;; Multiplication function: multiplies two i32 values
  (func $mul_func (type $t0)
    local.get 0
    local.get 1
    i32.mul
  )

  ;; Triple parameter function: computes a * b * c
  (func $triple_func (type $t1)
    local.get 0
    local.get 1
    i32.mul
    local.get 2
    i32.mul
  )

  ;; Subtraction function: subtracts second from first
  (func $sub_func (type $t0)
    local.get 0
    local.get 1
    i32.sub
  )

  ;; Constant function returning 42
  (func $const42_func (type $t2)
    i32.const 42
  )

  ;; Constant function returning 100
  (func $const100_func (type $t2)
    i32.const 100
  )

  ;; Recursive countdown function using return_call_indirect
  ;; Counts down from n to 0, returns 0
  (func $recursive_countdown (type $t3) (param $n i32) (result i32)
    local.get $n
    i32.const 0
    i32.eq
    if (result i32)
      i32.const 0
    else
      local.get $n
      i32.const 1
      i32.sub
      i32.const 6  ;; table index for recursive_countdown
      return_call_indirect (type $t3)
    end
  )

  ;; Recursive factorial function using return_call_indirect
  ;; Computes n! using tail recursion with accumulator
  (func $recursive_factorial (type $t0) (param $n i32) (param $acc i32) (result i32)
    local.get $n
    i32.const 1
    i32.le_s
    if (result i32)
      local.get $acc
    else
      local.get $n
      i32.const 1
      i32.sub
      local.get $acc
      local.get $n
      i32.mul
      i32.const 7  ;; table index for recursive_factorial
      return_call_indirect (type $t0)
    end
  )

  ;; Fibonacci function using return_call_indirect (iterative approach with accumulator)
  ;; Uses helper function for tail recursion
  (func $fibonacci (type $t1) (param $n i32) (param $a i32) (param $b i32) (result i32)
    local.get $n
    i32.const 0
    i32.eq
    if (result i32)
      local.get $a
    else
      local.get $n
      i32.const 1
      i32.sub
      local.get $b
      local.get $a
      local.get $b
      i32.add
      i32.const 8  ;; table index for fibonacci
      return_call_indirect (type $t1)
    end
  )

  ;; GCD function using return_call_indirect (Euclidean algorithm)
  (func $gcd (type $t0) (param $a i32) (param $b i32) (result i32)
    local.get $b
    i32.const 0
    i32.eq
    if (result i32)
      local.get $a
    else
      local.get $b
      local.get $a
      local.get $b
      i32.rem_u
      i32.const 9  ;; table index for gcd
      return_call_indirect (type $t0)
    end
  )

  ;; Test wrapper functions for C++ test cases

  ;; Test indirect addition through return_call_indirect
  (func $test_indirect_add (export "test_indirect_add") (type $t1) (param $a i32) (param $b i32) (param $table_idx i32) (result i32)
    local.get $a
    local.get $b
    local.get $table_idx
    return_call_indirect (type $t0)
  )

  ;; Test indirect multiplication through return_call_indirect
  (func $test_indirect_mul (export "test_indirect_mul") (type $t1) (param $a i32) (param $b i32) (param $table_idx i32) (result i32)
    local.get $a
    local.get $b
    local.get $table_idx
    return_call_indirect (type $t0)
  )

  ;; Additional type for 4-parameter functions
  (type $t4 (func (param i32 i32 i32 i32) (result i32)))

  ;; Test triple parameter function through return_call_indirect
  (func $test_triple_param (export "test_triple_param") (type $t4) (param $a i32) (param $b i32) (param $c i32) (param $table_idx i32) (result i32)
    local.get $a
    local.get $b
    local.get $c
    local.get $table_idx
    return_call_indirect (type $t1)
  )

  ;; Test indirect subtraction through return_call_indirect
  (func $test_indirect_sub (export "test_indirect_sub") (type $t1) (param $a i32) (param $b i32) (param $table_idx i32) (result i32)
    local.get $a
    local.get $b
    local.get $table_idx
    return_call_indirect (type $t0)
  )

  ;; Test void function (no parameters) through return_call_indirect
  (func $test_void_function (export "test_void_function") (type $t3) (param $table_idx i32) (result i32)
    local.get $table_idx
    return_call_indirect (type $t2)
  )

  ;; Test constant function through return_call_indirect
  (func $test_constant_function (export "test_constant_function") (type $t3) (param $table_idx i32) (result i32)
    local.get $table_idx
    return_call_indirect (type $t2)
  )

  ;; Test recursive countdown through return_call_indirect
  (func $test_recursive_countdown (export "test_recursive_countdown") (type $t0) (param $count i32) (param $table_idx i32) (result i32)
    local.get $count
    local.get $table_idx
    return_call_indirect (type $t3)
  )

  ;; Test recursive factorial through return_call_indirect
  (func $test_recursive_factorial (export "test_recursive_factorial") (type $t0) (param $n i32) (param $table_idx i32) (result i32)
    local.get $n
    i32.const 1  ;; initial accumulator
    local.get $table_idx
    return_call_indirect (type $t0)
  )

  ;; Test fibonacci calculation through return_call_indirect
  (func $test_fibonacci (export "test_fibonacci") (type $t0) (param $n i32) (param $table_idx i32) (result i32)
    local.get $n
    i32.const 0  ;; initial a = 0
    i32.const 1  ;; initial b = 1
    local.get $table_idx
    return_call_indirect (type $t1)
  )

  ;; Test GCD calculation through return_call_indirect
  (func $test_gcd (export "test_gcd") (type $t1) (param $a i32) (param $b i32) (param $table_idx i32) (result i32)
    local.get $a
    local.get $b
    local.get $table_idx
    return_call_indirect (type $t0)
  )
)