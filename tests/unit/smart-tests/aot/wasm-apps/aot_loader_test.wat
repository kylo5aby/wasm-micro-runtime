(module
  ;; Memory declaration for AOT loader testing
  (memory 1)
  
  ;; Global variables for import testing
  (global $test_global (mut i32) (i32.const 42))
  
  ;; Table for table initialization testing
  (table 10 funcref)
  
  ;; Data section for data relocation testing
  (data (i32.const 0) "test_data_for_relocation")
  
  ;; Function exports for AOT section loading
  (func (export "test_function") (result i32)
    (i32.const 123)
  )
  
  ;; Function with parameters for symbol resolution
  (func (export "test_with_params") (param $a i32) (param $b i32) (result i32)
    (i32.add (local.get $a) (local.get $b))
  )
  
  ;; Function that uses global variable
  (func (export "test_global_access") (result i32)
    (global.get $test_global)
  )
  
  ;; Function that uses memory
  (func (export "test_memory_access") (result i32)
    (i32.load (i32.const 0))
  )
  
  ;; Function for name section testing
  (func (export "named_function") (result i32)
    (i32.const 456)
  )
)