(module
  ;; Module with comprehensive import/export testing
  ;; Tests various import and export scenarios for module loading validation
  
  ;; Type section: Function signatures for imports and exports
  (type $math_func (func (param i32 i32) (result i32)))
  (type $print_func (func (param i32)))
  (type $getter_func (func (result i32)))
  (type $void_func (func))
  
  ;; Import section: Import functions, table, and globals
  (import "env" "print_int" (func $print_int (type $print_func)))
  (import "env" "get_time" (func $get_time (type $getter_func)))
  (import "env" "function_table" (table $imported_table 2 funcref))
  (import "env" "global_counter" (global $imported_counter (mut i32)))
  (import "env" "constant_value" (global $imported_const i32))
  
  ;; Function section: Internal functions that use imports
  (func $calculate (type $math_func) (param $a i32) (param $b i32) (result i32)
    ;; Use imported global
    global.get $imported_counter
    i32.const 1
    i32.add
    global.set $imported_counter
    
    ;; Perform calculation
    local.get $a
    local.get $b
    i32.add
    global.get $imported_const
    i32.mul
  )
  
  (func $process_data (type $void_func)
    ;; Use local memory for operations
    i32.const 0
    i32.const 42
    i32.store
    
    ;; Use imported function
    i32.const 0
    i32.load
    call $print_int
  )
  
  (func $get_timestamp (type $getter_func) (result i32)
    ;; Call imported function
    call $get_time
  )
  
  (func $table_operation (type $void_func)
    ;; Use imported table (indirect call)
    i32.const 0
    call_indirect $imported_table (type $void_func)
  )
  
  ;; Local memory and table for exports
  (memory 1 2)
  (table $local_table 3 funcref)
  
  ;; Local globals for exports
  (global $local_counter (mut i32) (i32.const 0))
  (global $local_const i32 (i32.const 100))
  
  ;; Export section: Export functions, memory, table, and globals
  (export "calculate" (func $calculate))
  (export "process_data" (func $process_data))
  (export "get_timestamp" (func $get_timestamp))
  (export "table_operation" (func $table_operation))
  
  ;; Export memory and table
  (export "local_memory" (memory 0))
  (export "local_table" (table $local_table))
  
  ;; Export globals
  (export "local_counter" (global $local_counter))
  (export "local_const" (global $local_const))
  
  ;; Initialize local table
  (elem (i32.const 0) $calculate $process_data $get_timestamp)
  
  ;; Initialize local memory
  (data (i32.const 0) "Import/Export Test")
)