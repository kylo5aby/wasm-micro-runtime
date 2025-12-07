(module
  ;; Multi-function test module for AOT testing
  ;; Memory: 1 page (64KB)
  (memory 1)
  
  ;; Export memory
  (export "memory" (memory 0))
  
  ;; Global variables for testing
  (global $counter (mut i32) (i32.const 0))
  (global $max_value i32 (i32.const 1000))
  
  ;; Export globals
  (export "counter" (global $counter))
  (export "max_value" (global $max_value))
  
  ;; Helper function: increment counter
  (func $increment_counter
    global.get $counter
    i32.const 1
    i32.add
    global.set $counter
  )
  
  ;; Helper function: reset counter
  (func $reset_counter
    i32.const 0
    global.set $counter
  )
  
  ;; Exported function: get counter value
  (func $get_counter (result i32)
    global.get $counter
  )
  (export "get_counter" (func $get_counter))
  
  ;; Exported function: set counter value
  (func $set_counter (param $value i32)
    local.get $value
    global.set $counter
  )
  (export "set_counter" (func $set_counter))
  
  ;; Factorial function (recursive)
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
    end
  )
  (export "factorial" (func $factorial))
  
  ;; Fibonacci function (recursive)
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
    end
  )
  (export "fibonacci" (func $fibonacci))
  
  ;; Function with multiple parameters and local variables
  (func $complex_calculation (param $a i32) (param $b i32) (param $c i32) (param $d i32) (result i32)
    (local $temp1 i32)
    (local $temp2 i32)
    (local $result i32)
    
    ;; temp1 = a * b
    local.get $a
    local.get $b
    i32.mul
    local.set $temp1
    
    ;; temp2 = c + d
    local.get $c
    local.get $d
    i32.add
    local.set $temp2
    
    ;; result = temp1 - temp2
    local.get $temp1
    local.get $temp2
    i32.sub
    local.set $result
    
    ;; Increment counter as side effect
    call $increment_counter
    
    local.get $result
  )
  (export "complex_calculation" (func $complex_calculation))
  
  ;; Function that calls multiple other functions
  (func $call_multiple_functions (param $n i32) (result i32)
    (local $fact_result i32)
    (local $fib_result i32)
    
    ;; Reset counter
    call $reset_counter
    
    ;; Calculate factorial
    local.get $n
    call $factorial
    local.set $fact_result
    
    ;; Calculate fibonacci
    local.get $n
    call $fibonacci
    local.set $fib_result
    
    ;; Return sum
    local.get $fact_result
    local.get $fib_result
    i32.add
  )
  (export "call_multiple_functions" (func $call_multiple_functions))
  
  ;; Function with loops
  (func $sum_range (param $start i32) (param $end i32) (result i32)
    (local $i i32)
    (local $sum i32)
    
    local.get $start
    local.set $i
    i32.const 0
    local.set $sum
    
    (loop $loop
      local.get $i
      local.get $end
      i32.gt_s
      if
        br 1
      end
      
      local.get $sum
      local.get $i
      i32.add
      local.set $sum
      
      local.get $i
      i32.const 1
      i32.add
      local.set $i
      br $loop
    )
    
    local.get $sum
  )
  (export "sum_range" (func $sum_range))
  
  ;; Function with conditional branching
  (func $max_of_four (param $a i32) (param $b i32) (param $c i32) (param $d i32) (result i32)
    (local $max i32)
    
    local.get $a
    local.set $max
    
    local.get $b
    local.get $max
    i32.gt_s
    if
      local.get $b
      local.set $max
    end
    
    local.get $c
    local.get $max
    i32.gt_s
    if
      local.get $c
      local.set $max
    end
    
    local.get $d
    local.get $max
    i32.gt_s
    if
      local.get $d
      local.set $max
    end
    
    local.get $max
  )
  (export "max_of_four" (func $max_of_four))
  
  ;; Function that tests floating point operations
  (func $float_operations (param $x f64) (param $y f64) (result f64)
    (local $temp f64)
    
    ;; (x + y) * (x - y) = x^2 - y^2
    local.get $x
    local.get $y
    f64.add
    local.set $temp
    
    local.get $temp
    local.get $x
    local.get $y
    f64.sub
    f64.mul
  )
  (export "float_operations" (func $float_operations))
  
  ;; Function that might cause stack overflow if called recursively too much
  (func $deep_recursion (param $depth i32) (result i32)
    local.get $depth
    i32.const 0
    i32.le_s
    if (result i32)
      i32.const 1
    else
      local.get $depth
      i32.const 1
      i32.sub
      call $deep_recursion
      local.get $depth
      i32.add
    end
  )
  (export "deep_recursion" (func $deep_recursion))
)