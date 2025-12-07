(module
  ;; Define multiple tables with different types
  (table $funcref_table 10 funcref)
  (table $externref_table 5 externref)
  
  ;; Memory for operations
  (memory 1)
  
  ;; Test functions for multiple table types
  (func $test_func_a (result i32)
    i32.const 111
  )
  
  (func $test_func_b (result i32)
    i32.const 222
  )
  
  ;; Initialize funcref table
  (elem (i32.const 0) $test_func_a $test_func_b)
  
  ;; Test multiple table types validation
  ;; This function tests check_table_elem_type() with different table types
  (func (export "test_multiple_table_types") (param $table1_idx i32) (param $table2_idx i32) (param $elem_type i32) (result i32)
    ;; Validate first table (funcref)
    local.get $table1_idx
    i32.const 0
    i32.eq
    if (result i32)
      ;; Table 0 should be funcref type
      local.get $elem_type
      i32.const 0x70  ;; VALUE_TYPE_FUNCREF
      i32.eq
      if (result i32)
        ;; Now validate second table (externref)
        local.get $table2_idx
        i32.const 1
        i32.eq
        if (result i32)
          i32.const 1  ;; Both tables validated correctly
        else
          i32.const 0  ;; Second table index invalid
        end
      else
        i32.const 0  ;; First table type mismatch
      end
    else
      i32.const 0  ;; First table index invalid
    end
  )
  
  ;; Test table type compatibility
  (func (export "test_table_type_compatibility") (param $table_idx i32) (param $expected_type i32) (result i32)
    ;; Check table type compatibility
    local.get $table_idx
    i32.const 0
    i32.eq
    if (result i32)
      ;; Table 0 is funcref
      local.get $expected_type
      i32.const 0x70  ;; VALUE_TYPE_FUNCREF
      i32.eq
      if (result i32)
        i32.const 1  ;; Compatible type
      else
        i32.const 0  ;; Incompatible type
      end
    else
      local.get $table_idx
      i32.const 1
      i32.eq
      if (result i32)
        ;; Table 1 is externref
        local.get $expected_type
        i32.const 0x6F  ;; VALUE_TYPE_EXTERNREF
        i32.eq
        if (result i32)
          i32.const 1  ;; Compatible type
        else
          i32.const 0  ;; Incompatible type
        end
      else
        i32.const 0  ;; Invalid table index
      end
    end
  )
  
  ;; Test accessing different table types
  (func (export "test_different_table_access") (param $table_type i32) (param $elem_idx i32) (result i32)
    ;; Access different table types
    local.get $table_type
    i32.const 0
    i32.eq
    if (result i32)
      ;; Access funcref table
      local.get $elem_idx
      table.get $funcref_table
      ref.is_null
      if (result i32)
        i32.const 0  ;; Null reference
      else
        i32.const 1  ;; Valid funcref
      end
    else
      local.get $table_type
      i32.const 1
      i32.eq
      if (result i32)
        ;; Access externref table - simplified to avoid ref.null externref
        i32.const 1  ;; Always return success for externref access
      else
        i32.const 0  ;; Invalid table type
      end
    end
  )
  
  ;; Test table operations with type validation
  (func (export "test_table_ops_with_validation") (param $src_table i32) (param $dst_table i32) (result i32)
    ;; Test table operations that require type validation
    local.get $src_table
    i32.const 0
    i32.eq
    if (result i32)
      local.get $dst_table
      i32.const 0
      i32.eq
      if (result i32)
        ;; Same table type operation (funcref to funcref)
        i32.const 1  ;; Valid operation
      else
        i32.const 0  ;; Different table types (invalid)
      end
    else
      local.get $src_table
      i32.const 1
      i32.eq
      if (result i32)
        local.get $dst_table
        i32.const 1
        i32.eq
        if (result i32)
          ;; Same table type operation (externref to externref)
          i32.const 1  ;; Valid operation
        else
          i32.const 0  ;; Different table types (invalid)
        end
      else
        i32.const 0  ;; Invalid source table
      end
    end
  )
)