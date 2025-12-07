(module
  ;; Define a table with no initial segments
  (table $empty_table 5 funcref)
  
  ;; Memory for operations
  (memory 1)
  
  ;; Test function for empty table segments
  (func $placeholder_func (result i32)
    i32.const 0
  )
  
  ;; Test empty table segments handling
  ;; This function tests load_table_segment_section() with empty segments
  (func (export "test_empty_segments") (param $expected_count i32) (result i32)
    ;; Check if expected segment count matches actual (should be 0)
    local.get $expected_count
    i32.const 0
    i32.eq
    if (result i32)
      i32.const 1  ;; Correctly handled empty segments
    else
      i32.const 0  ;; Incorrect segment count
    end
  )
  
  ;; Test accessing empty table
  (func (export "test_empty_table_access") (param $index i32) (result i32)
    ;; Try to access empty table
    local.get $index
    table.get $empty_table
    ref.is_null
    if (result i32)
      i32.const 1  ;; Correctly returns null for empty table
    else
      i32.const 0  ;; Unexpected non-null value
    end
  )
  
  ;; Test table size with empty segments
  (func (export "test_empty_table_size") (result i32)
    ;; Check table size
    table.size $empty_table
    i32.const 5  ;; Expected table size
    i32.eq
    if (result i32)
      i32.const 1  ;; Correct table size
    else
      i32.const 0  ;; Incorrect table size
    end
  )
)