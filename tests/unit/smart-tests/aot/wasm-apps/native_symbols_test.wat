(module
  ;; Import native functions for symbol resolution testing
  (import "env" "native_function_i32" (func $native_i32 (param i32) (result i32)))
  (import "env" "native_function_i64" (func $native_i64 (param i64) (result i64)))
  (import "env" "native_function_f32" (func $native_f32 (param f32) (result f32)))
  (import "env" "native_function_f64" (func $native_f64 (param f64) (result f64)))
  (import "env" "native_print" (func $native_print (param i32 i32)))
  
  ;; Memory for string operations
  (memory 1)
  (data (i32.const 0) "Test message for native print")
  
  ;; Export functions that call native symbols
  (func (export "test_native_i32") (param $value i32) (result i32)
    (call $native_i32 (local.get $value))
  )
  
  (func (export "test_native_i64") (param $value i64) (result i64)
    (call $native_i64 (local.get $value))
  )
  
  (func (export "test_native_f32") (param $value f32) (result f32)
    (call $native_f32 (local.get $value))
  )
  
  (func (export "test_native_f64") (param $value f64) (result f64)
    (call $native_f64 (local.get $value))
  )
  
  (func (export "test_native_print")
    (call $native_print (i32.const 0) (i32.const 29))
  )
  
  ;; Function for testing symbol name resolution
  (func (export "symbol_resolution_test") (result i32)
    (i32.const 42)
  )
)