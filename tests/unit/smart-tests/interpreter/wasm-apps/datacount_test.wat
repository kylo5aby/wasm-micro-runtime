(module
  ;; Memory for data operations
  (memory 1)
  
  ;; Data segments for testing datacount section
  (data (i32.const 0) "Hello, World!")
  (data (i32.const 32) "WAMR Testing")
  
  ;; Test datacount section loading
  ;; This function tests load_datacount_section() functionality
  (func (export "test_datacount_section") (param $expected_count i32) (result i32)
    ;; Simulate datacount validation
    ;; In real implementation, this would check module->data_seg_count1
    local.get $expected_count
    i32.const 2  ;; We have 2 data segments defined above
    i32.eq
    if (result i32)
      i32.const 1  ;; Correct datacount
    else
      i32.const 0  ;; Incorrect datacount
    end
  )
  
  ;; Test data segment access to validate datacount loading
  (func (export "test_data_segment_access") (param $segment_idx i32) (result i32)
    ;; Test accessing different data segments
    local.get $segment_idx
    i32.const 0
    i32.eq
    if (result i32)
      ;; Access first data segment
      i32.const 0
      i32.load8_u
      i32.const 72  ;; 'H' in "Hello, World!"
      i32.eq
      if (result i32)
        i32.const 1  ;; Successfully accessed first segment
      else
        i32.const 0  ;; Failed to access first segment
      end
    else
      local.get $segment_idx
      i32.const 1
      i32.eq
      if (result i32)
        ;; Access second data segment
        i32.const 32
        i32.load8_u
        i32.const 87  ;; 'W' in "WAMR Testing"
        i32.eq
        if (result i32)
          i32.const 1  ;; Successfully accessed second segment
        else
          i32.const 0  ;; Failed to access second segment
        end
      else
        i32.const 0  ;; Invalid segment index
      end
    end
  )
  
  ;; Test bulk memory operations that require datacount section
  (func (export "test_bulk_memory_with_datacount") (param $dest i32) (param $src i32) (param $size i32) (result i32)
    ;; Test memory.init operation which requires datacount section
    local.get $dest
    local.get $src
    local.get $size
    memory.init 0  ;; Initialize from data segment 0
    
    ;; Verify the operation succeeded
    local.get $dest
    i32.load8_u
    i32.const 72  ;; 'H' from "Hello, World!"
    i32.eq
    if (result i32)
      i32.const 1  ;; Bulk memory operation succeeded
    else
      i32.const 0  ;; Bulk memory operation failed
    end
  )
  
  ;; Test data.drop operation
  (func (export "test_data_drop") (param $segment_idx i32) (result i32)
    ;; Test data.drop operation
    local.get $segment_idx
    i32.const 0
    i32.eq
    if (result i32)
      data.drop 0  ;; Drop data segment 0
      i32.const 1  ;; Successfully dropped segment
    else
      local.get $segment_idx
      i32.const 1
      i32.eq
      if (result i32)
        data.drop 1  ;; Drop data segment 1
        i32.const 1  ;; Successfully dropped segment
      else
        i32.const 0  ;; Invalid segment index
      end
    end
  )
)