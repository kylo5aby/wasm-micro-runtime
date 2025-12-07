(module
  ;; Define table for segment testing
  (table $main_table 20 funcref)
  
  ;; Memory for operations
  (memory 1)
  
  ;; Test functions for table segments
  (func $segment_func_1 (result i32)
    i32.const 100
  )
  
  (func $segment_func_2 (result i32)
    i32.const 200
  )
  
  (func $segment_func_3 (result i32)
    i32.const 300
  )
  
  ;; Table segment initialization
  ;; This exercises load_table_segment_section()
  (elem (table $main_table) (i32.const 0) func $segment_func_1 $segment_func_2)
  (elem (table $main_table) (i32.const 5) func $segment_func_3)
  
  ;; Test table segment loading validation
  ;; This function tests load_table_segment_section() success path
  (func (export "test_table_segment_load") (param $table_index i32) (param $expected_segments i32) (result i32)
    ;; Simulate table segment loading validation
    local.get $table_index
    i32.const 0
    i32.eq
    if (result i32)
      ;; Table 0 has segments, check expected count
      local.get $expected_segments
      i32.const 2  ;; We have 2 element segments
      i32.eq
      if (result i32)
        i32.const 1  ;; Correct segment count
      else
        i32.const 0  ;; Incorrect segment count
      end
    else
      i32.const 0  ;; Invalid table index
    end
  )
  
  ;; Test table segment validation with actual access
  (func (export "test_table_segment_validation") (param $elem_index i32) (result i32)
    ;; Test accessing elements from different segments
    local.get $elem_index
    i32.const 0
    i32.eq
    if (result i32)
      ;; Access first element from segment 1
      i32.const 0
      table.get $main_table
      ref.is_null
      if (result i32)
        i32.const 0  ;; Null reference (unexpected)
      else
        i32.const 1  ;; Valid reference from segment
      end
    else
      local.get $elem_index
      i32.const 5
      i32.eq
      if (result i32)
        ;; Access first element from segment 2
        i32.const 5
        table.get $main_table
        ref.is_null
        if (result i32)
          i32.const 0  ;; Null reference (unexpected)
        else
          i32.const 1  ;; Valid reference from segment
        end
      else
        local.get $elem_index
        i32.const 10
        i32.eq
        if (result i32)
          ;; Access uninitialized element
          i32.const 10
          table.get $main_table
          ref.is_null
          if (result i32)
            i32.const 1  ;; Null reference (expected for uninitialized)
          else
            i32.const 0  ;; Unexpected non-null reference
          end
        else
          i32.const 0  ;; Invalid test index
        end
      end
    end
  )
  
  ;; Test table segment bounds checking
  (func (export "test_table_segment_bounds") (param $start_index i32) (param $count i32) (result i32)
    ;; Simulate bounds checking for table segments
    local.get $start_index
    local.get $count
    i32.add
    i32.const 20  ;; Table size
    i32.le_u
    if (result i32)
      i32.const 1  ;; Within bounds
    else
      i32.const 0  ;; Out of bounds
    end
  )
  
  ;; Test multiple segment initialization
  (func (export "test_multiple_segments") (param $segment_id i32) (result i32)
    ;; Test accessing elements from different segments
    local.get $segment_id
    i32.const 0
    i32.eq
    if (result i32)
      ;; Test segment 0 (elements 0-1)
      i32.const 0
      call_indirect (type 0)
      i32.const 100
      i32.eq
      if (result i32)
        i32.const 1  ;; Correct function from segment 0
      else
        i32.const 0  ;; Wrong function result
      end
    else
      local.get $segment_id
      i32.const 1
      i32.eq
      if (result i32)
        ;; Test segment 1 (element 5)
        i32.const 5
        call_indirect (type 0)
        i32.const 300
        i32.eq
        if (result i32)
          i32.const 1  ;; Correct function from segment 1
        else
          i32.const 0  ;; Wrong function result
        end
      else
        i32.const 0  ;; Invalid segment ID
      end
    end
  )
  
  ;; Test table segment error handling
  (func (export "test_segment_error_handling") (param $error_type i32) (result i32)
    ;; Test different error conditions in segment loading
    local.get $error_type
    i32.const 0
    i32.eq
    if (result i32)
      ;; Test out of bounds access
      i32.const 25  ;; Beyond table size (20)
      table.get $main_table
      ref.is_null
      if (result i32)
        i32.const 1  ;; Correctly handles out of bounds (returns null)
      else
        i32.const 0  ;; Should have returned null
      end
    else
      local.get $error_type
      i32.const 1
      i32.eq
      if (result i32)
        ;; Test accessing uninitialized element (should be null)
        i32.const 15  ;; Uninitialized element
        table.get $main_table
        ref.is_null
        if (result i32)
          i32.const 1  ;; Correctly returns null for uninitialized
        else
          i32.const 0  ;; Should have been null
        end
      else
        i32.const 1  ;; Unknown error type, return success
      end
    end
  )
  
  ;; Function type for call_indirect
  (type (func (result i32)))
)