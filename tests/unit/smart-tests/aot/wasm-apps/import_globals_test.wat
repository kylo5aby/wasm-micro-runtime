(module
  ;; Import global variables for testing load_import_globals function
  (import "env" "imported_i32" (global $imported_i32 i32))
  (import "env" "imported_i64" (global $imported_i64 i64))
  (import "env" "imported_f32" (global $imported_f32 f32))
  (import "env" "imported_f64" (global $imported_f64 f64))
  (import "env" "imported_mut_i32" (global $imported_mut_i32 (mut i32)))
  
  ;; Memory for testing
  (memory 1)
  
  ;; Functions that use imported globals
  (func (export "get_imported_i32") (result i32)
    (global.get $imported_i32)
  )
  
  (func (export "get_imported_i64") (result i64)
    (global.get $imported_i64)
  )
  
  (func (export "get_imported_f32") (result f32)
    (global.get $imported_f32)
  )
  
  (func (export "get_imported_f64") (result f64)
    (global.get $imported_f64)
  )
  
  (func (export "get_imported_mut_i32") (result i32)
    (global.get $imported_mut_i32)
  )
  
  (func (export "set_imported_mut_i32") (param $value i32)
    (global.set $imported_mut_i32 (local.get $value))
  )
)