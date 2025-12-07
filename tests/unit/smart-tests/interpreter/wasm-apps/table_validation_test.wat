(module
  ;; Define table with funcref elements
  (table $t0 10 funcref)
  (table $t1 5 externref)
  
  ;; Memory for operations
  (memory 1)
  
  ;; Test functions for table validation
  (func $dummy_func (result i32)
    i32.const 42
  )
  
  ;; Initialize table with function references
  (elem (i32.const 0) $dummy_func)
  
  ;; Test valid table element type validation
  ;; This function tests check_table_elem_type() with matching types
  (func (export "test_valid_table_elem_type") (param $table_index i32) (param $elem_type i32) (result i32)
    ;; Simulate table element type checking
    local.get $table_index
    i32.const 0
    i32.eq
    if (result i32)
      ;; Table 0 expects funcref (0x70)
      local.get $elem_type
      i32.const 0x70  ;; VALUE_TYPE_FUNCREF
      i32.eq
      if (result i32)
        i32.const 1  ;; Valid type match
      else
        i32.const 0  ;; Invalid type mismatch
      end
    else
      local.get $table_index
      i32.const 1
      i32.eq
      if (result i32)
        ;; Table 1 expects externref (0x6F)
        local.get $elem_type
        i32.const 0x6F  ;; VALUE_TYPE_EXTERNREF
        i32.eq
        if (result i32)
          i32.const 1  ;; Valid type match
        else
          i32.const 0  ;; Invalid type mismatch
        end
      else
        i32.const 0  ;; Invalid table index
      end
    end
  )
  
  ;; Test invalid table element type validation
  ;; This function tests check_table_elem_type() with mismatched types
  (func (export "test_invalid_table_elem_type") (param $table_index i32) (param $elem_type i32) (result i32)
    ;; Simulate invalid type checking
    local.get $table_index
    i32.const 0
    i32.eq
    if (result i32)
      ;; Table 0 expects funcref but we test with invalid type
      local.get $elem_type
      i32.const 0x7F  ;; Invalid type
      i32.eq
      if (result i32)
        i32.const 0  ;; Correctly detected invalid type
      else
        i32.const 1  ;; Should detect mismatch
      end
    else
      i32.const 0  ;; Invalid table index
    end
  )
  
  ;; Test valid table index validation
  ;; This function tests check_table_index() with valid indices
  (func (export "test_valid_table_index") (param $table_index i32) (result i32)
    ;; Check if table index is within bounds (0-1 for our 2 tables)
    local.get $table_index
    i32.const 2  ;; Total table count
    i32.lt_u
    if (result i32)
      i32.const 1  ;; Valid index
    else
      i32.const 0  ;; Invalid index
    end
  )
  
  ;; Test invalid table index validation
  ;; This function tests check_table_index() with invalid indices
  (func (export "test_invalid_table_index") (param $table_index i32) (result i32)
    ;; Check if table index is out of bounds
    local.get $table_index
    i32.const 999
    i32.eq
    if (result i32)
      i32.const 0  ;; Correctly detected invalid index
    else
      local.get $table_index
      i32.const 2  ;; Total table count
      i32.ge_u
      if (result i32)
        i32.const 0  ;; Invalid index (out of bounds)
      else
        i32.const 1  ;; Valid index
      end
    end
  )
  
  ;; Test table operations with validation
  (func (export "test_table_operations") (param $table_idx i32) (param $elem_idx i32) (result i32)
    ;; Test table.get operation which triggers table validation
    local.get $table_idx
    i32.const 0
    i32.eq
    if (result i32)
      ;; Access table 0
      local.get $elem_idx
      table.get $t0
      ref.is_null
      if (result i32)
        i32.const 0  ;; Null reference
      else
        i32.const 1  ;; Valid reference
      end
    else
      i32.const 0  ;; Invalid table or operation failed
    end
  )
)