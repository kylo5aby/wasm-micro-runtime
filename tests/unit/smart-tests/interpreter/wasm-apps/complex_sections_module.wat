(module
  ;; Complex module with multiple sections for comprehensive testing
  ;; Tests type, function, memory, table, global, export, code, and data sections
  
  ;; Type section: Multiple function signatures
  (type $binary_op (func (param i32 i32) (result i32)))
  (type $unary_op (func (param i32) (result i32)))
  (type $void_func (func))
  (type $memory_func (func (param i32 i32)))
  
  ;; Import section: Import external functions
  (import "env" "external_func" (func $external (type $unary_op)))
  
  ;; Function section: Declare internal functions
  (func $add (type $binary_op) (param $a i32) (param $b i32) (result i32)
    local.get $a
    local.get $b
    i32.add
  )
  
  (func $multiply (type $binary_op) (param $a i32) (param $b i32) (result i32)
    local.get $a
    local.get $b
    i32.mul
  )
  
  (func $increment (type $unary_op) (param $x i32) (result i32)
    local.get $x
    i32.const 1
    i32.add
  )
  
  (func $init_globals (type $void_func)
    i32.const 100
    global.set $counter
    
    f64.const 3.14159
    global.set $pi
  )
  
  (func $memory_operations (type $memory_func) (param $addr i32) (param $value i32)
    ;; Store value at address
    local.get $addr
    local.get $value
    i32.store
    
    ;; Load and verify
    local.get $addr
    i32.load
    drop
  )
  
  ;; Table section: Function reference table
  (table $func_table 4 funcref)
  
  ;; Memory section: Linear memory with initial size
  (memory $main_memory 2 10)  ;; Initial: 2 pages, Max: 10 pages
  
  ;; Global section: Mutable and immutable globals
  (global $counter (mut i32) (i32.const 0))
  (global $pi (mut f64) (f64.const 0.0))
  (global $max_size i32 (i32.const 1024))
  
  ;; Export section: Export functions, memory, table, and globals
  (export "add" (func $add))
  (export "multiply" (func $multiply))
  (export "increment" (func $increment))
  (export "init_globals" (func $init_globals))
  (export "memory_operations" (func $memory_operations))
  (export "memory" (memory $main_memory))
  (export "func_table" (table $func_table))
  (export "counter" (global $counter))
  (export "max_size" (global $max_size))
  
  ;; Element section: Initialize function table
  (elem (i32.const 0) $add $multiply $increment $init_globals)
  
  ;; Data section: Initialize memory with test data
  (data (i32.const 0) "WAMR Test Data")
  (data (i32.const 100) "Complex Module")
  (data (i32.const 200) "\00\01\02\03\04\05\06\07\08\09")
  
  ;; Start section: Initialization function
  (start $init_globals)
)