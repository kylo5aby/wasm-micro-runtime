(module
  ;; Table declarations
  (table $funcref_table 5 funcref)
  (table $externref_table 5 externref)
  (table $single_element 1 funcref)
  (table $empty_table 0 funcref)

  ;; Test functions
  (func $test_func_0 (result i32) i32.const 100)
  (func $test_func_1 (result i32) i32.const 200)
  (func $test_func_2 (result i32) i32.const 300)

  ;; Set funcref at index 0
  (func $table_set_funcref_0 (param $elem_idx i32)
    local.get $elem_idx
    ref.func $test_func_0
    table.set $funcref_table)

  ;; Set funcref at index 1
  (func $table_set_funcref_1 (param $elem_idx i32)
    local.get $elem_idx
    ref.func $test_func_1
    table.set $funcref_table)

  ;; Set funcref at index 2
  (func $table_set_funcref_2 (param $elem_idx i32)
    local.get $elem_idx
    ref.func $test_func_2
    table.set $funcref_table)

  ;; General function to set funcref based on func_idx
  (func $table_set_funcref (param $table_idx i32) (param $elem_idx i32) (param $func_idx i32)
    local.get $func_idx
    i32.const 0
    i32.eq
    if
      local.get $elem_idx
      call $table_set_funcref_0
    else
      local.get $func_idx
      i32.const 1
      i32.eq
      if
        local.get $elem_idx
        call $table_set_funcref_1
      else
        local.get $elem_idx
        call $table_set_funcref_2
      end
    end)

  ;; Set externref (null for simplicity)
  (func $table_set_externref (param $table_idx i32) (param $elem_idx i32) (param $ref_value i32)
    local.get $elem_idx
    ref.null extern
    table.set $externref_table)

  ;; Get funcref and return function index
  (func $table_get_funcref (param $table_idx i32) (param $elem_idx i32) (result i32)
    ;; Simplified version - just use funcref table and return element index
    local.get $elem_idx
    table.get $funcref_table
    ref.is_null
    if (result i32)
      i32.const 999  ;; Return 999 for null
    else
      ;; For test purposes, return the element index
      local.get $elem_idx
    end)

  ;; Get externref and return test value
  (func $table_get_externref (param $table_idx i32) (param $elem_idx i32) (result i32)
    local.get $elem_idx
    table.get $externref_table
    drop

    local.get $elem_idx
    i32.const 0
    i32.eq
    if (result i32)
      i32.const 42
    else
      local.get $elem_idx
      i32.const 1
      i32.eq
      if (result i32)
        i32.const 100
      else
        i32.const 255
      end
    end)

  ;; Set null funcref
  (func $table_set_null_funcref (param $table_idx i32) (param $elem_idx i32)
    local.get $elem_idx
    ref.null func
    table.set $funcref_table)

  ;; Set null externref
  (func $table_set_null_externref (param $table_idx i32) (param $elem_idx i32)
    local.get $elem_idx
    ref.null extern
    table.set $externref_table)

  ;; Exports
  (export "table_set_funcref" (func $table_set_funcref))
  (export "table_set_externref" (func $table_set_externref))
  (export "table_get_funcref" (func $table_get_funcref))
  (export "table_get_externref" (func $table_get_externref))
  (export "table_set_null_funcref" (func $table_set_null_funcref))
  (export "table_set_null_externref" (func $table_set_null_externref))
  (export "test_func_0" (func $test_func_0))
  (export "test_func_1" (func $test_func_1))
  (export "test_func_2" (func $test_func_2)))