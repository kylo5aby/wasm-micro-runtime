(module
  ;; Simple function test module for AOT loading tests
  ;; Memory: 1 page (64KB)
  (memory 1)
  
  ;; Export memory for external access
  (export "memory" (memory 0))
  
  ;; Simple addition function
  (func $add (param $a i32) (param $b i32) (result i32)
    local.get $a
    local.get $b
    i32.add
  )
  (export "add" (func $add))
  
  ;; Simple multiplication function  
  (func $mul (param $a i32) (param $b i32) (result i32)
    local.get $a
    local.get $b
    i32.mul
  )
  (export "mul" (func $mul))
  
  ;; Function with no parameters, returns constant
  (func $get_constant (result i32)
    i32.const 42
  )
  (export "get_constant" (func $get_constant))
  
  ;; Function that accesses memory
  (func $store_and_load (param $addr i32) (param $value i32) (result i32)
    ;; Store value at address
    local.get $addr
    local.get $value
    i32.store
    
    ;; Load and return value from address
    local.get $addr
    i32.load
  )
  (export "store_and_load" (func $store_and_load))
)