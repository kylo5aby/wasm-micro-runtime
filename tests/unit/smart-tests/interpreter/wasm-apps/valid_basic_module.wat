(module
  ;; Basic valid WebAssembly module for testing module loading
  ;; Contains minimal required sections for a valid module
  
  ;; Memory section: 1 page (64KB) of linear memory
  (memory 1)
  
  ;; Type section: Define function signatures
  (type $add_type (func (param i32 i32) (result i32)))
  (type $void_type (func))
  
  ;; Function section: Declare functions
  (func $add (type $add_type) (param $a i32) (param $b i32) (result i32)
    local.get $a
    local.get $b
    i32.add
  )
  
  (func $test_memory (type $void_type)
    ;; Simple memory operation for testing
    i32.const 0
    i32.const 42
    i32.store
  )
  
  ;; Export section: Make functions available
  (export "add" (func $add))
  (export "test_memory" (func $test_memory))
  (export "memory" (memory 0))
  
  ;; Data section: Initialize memory with test data
  (data (i32.const 100) "Hello, WAMR!")
)