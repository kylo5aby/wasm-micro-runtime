(module
  ;; Function type definitions
  (type $void_func (func))
  (type $size_func (func (result i32)))
  (type $growth_func (func (result i32)))

  ;; Test function definitions (placeholder for initialization)
  (func $dummy_func (type $void_func))
  (func $init_func (type $void_func))

  ;; Tables for testing table.size opcode
  ;; Table 0: funcref table with initial size 10
  (table $funcref_table 10 funcref)

  ;; Table 1: externref table with initial size 5
  (table $externref_table 5 externref)

  ;; Table 2: empty table with initial size 0, maximum size 10
  (table $empty_table 0 10 funcref)

  ;; Initialize funcref table with function references
  (elem (table $funcref_table) (i32.const 0) $dummy_func $init_func $dummy_func $init_func $dummy_func
                                               $init_func $dummy_func $init_func $dummy_func $init_func)

  ;; Function to get funcref table size (table 0)
  (func $get_funcref_table_size (type $size_func)
    (result i32)
    table.size $funcref_table
  )

  ;; Function to get externref table size (table 1)
  (func $get_externref_table_size (type $size_func)
    (result i32)
    table.size $externref_table
  )

  ;; Function to get empty table size (table 2)
  (func $get_empty_table_size (type $size_func)
    (result i32)
    table.size $empty_table
  )

  ;; Function to grow funcref table by 15 elements and return original size
  (func $grow_funcref_table_by_15 (type $growth_func)
    (result i32)
    ref.null func
    i32.const 15
    table.grow $funcref_table
  )

  ;; Function to grow empty table by 3 elements and return original size
  (func $grow_empty_table_by_3 (type $growth_func)
    (result i32)
    ref.null func
    i32.const 3
    table.grow $empty_table
  )

  ;; Functions for multiple table testing
  ;; Get size of table 0 (funcref_table)
  (func $get_table_0_size (type $size_func)
    (result i32)
    table.size 0
  )

  ;; Get size of table 1 (externref_table)
  (func $get_table_1_size (type $size_func)
    (result i32)
    table.size 1
  )

  ;; Get size of table 2 (empty_table)
  (func $get_table_2_size (type $size_func)
    (result i32)
    table.size 2
  )

  ;; Grow table 1 (externref_table) by 7 elements
  (func $grow_table_1_by_7 (type $growth_func)
    (result i32)
    ref.null extern
    i32.const 7
    table.grow 1
  )

  ;; Export test functions for C++ test access
  (export "get_funcref_table_size" (func $get_funcref_table_size))
  (export "get_externref_table_size" (func $get_externref_table_size))
  (export "get_empty_table_size" (func $get_empty_table_size))
  (export "grow_funcref_table_by_15" (func $grow_funcref_table_by_15))
  (export "grow_empty_table_by_3" (func $grow_empty_table_by_3))
  (export "get_table_0_size" (func $get_table_0_size))
  (export "get_table_1_size" (func $get_table_1_size))
  (export "get_table_2_size" (func $get_table_2_size))
  (export "grow_table_1_by_7" (func $grow_table_1_by_7))
)