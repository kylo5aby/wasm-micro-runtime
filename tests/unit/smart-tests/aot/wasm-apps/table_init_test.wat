(module
  ;; Memory for data initialization testing
  (memory 1)
  
  ;; Data segment for testing data relocation
  (data (i32.const 0) "Hello, World!")
  (data (i32.const 16) "Test data for relocation")
  
  (func (export "get_data_at_offset") (param $offset i32) (result i32)
    (i32.load8_u (local.get $offset))
  )
  
  ;; Simple functions for testing without tables
  (func (export "get_table_size") (result i32)
    (i32.const 5)
  )
  
  (func (export "call_table_func") (param $index i32) (result i32)
    ;; Return different values based on index without using tables
    (local.get $index)
    (i32.const 0)
    (i32.eq)
    (if (result i32)
      (then (i32.const 100))
      (else
        (local.get $index)
        (i32.const 1)
        (i32.eq)
        (if (result i32)
          (then (i32.const 200))
          (else (i32.const 300))
        )
      )
    )
  )
)