(module
  ;; Function type definition
  (type $size_func (func (result i32)))

  ;; Only one table defined (index 0)
  (table $single_table 5 funcref)

  ;; Function that references valid table index 0
  ;; (We'll test invalid indices through manual bytecode manipulation)
  (func $valid_table_size (type $size_func)
    (result i32)
    table.size 0
  )

  ;; Export function for testing
  (export "valid_table_size" (func $valid_table_size))
)