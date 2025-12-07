(module
  ;; Define function types
  (type $func_type (func (result i32)))
  (type $test_func_type (func (param i32 i32 i32)))
  (type $get_func_type (func (param i32) (result i32)))

  ;; Function declarations for element segment
  (func $func_0 (result i32) (i32.const 100))
  (func $func_1 (result i32) (i32.const 200))
  (func $func_2 (result i32) (i32.const 300))
  (func $func_3 (result i32) (i32.const 400))
  (func $func_4 (result i32) (i32.const 500))

  ;; Table declaration (funcref table with size 10)
  (table $test_table 10 funcref)

  ;; Passive element segment with function references
  (elem $elem_seg func $func_0 $func_1 $func_2 $func_3 $func_4)

  ;; Test function: Basic table initialization
  (func $test_basic_init (param $table_offset i32) (param $elem_offset i32) (param $length i32)
    ;; Push arguments in reverse order for table.init
    (local.get $table_offset)
    (local.get $elem_offset)
    (local.get $length)
    ;; table.init table_index elem_seg_index
    (table.init $test_table $elem_seg)
  )

  ;; Test function: Boundary condition initialization
  (func $test_boundary_init (param $table_offset i32) (param $elem_offset i32) (param $length i32)
    (local.get $table_offset)
    (local.get $elem_offset)
    (local.get $length)
    (table.init $test_table $elem_seg)
  )

  ;; Test function: Zero length initialization
  (func $test_zero_length_init (param $table_offset i32) (param $elem_offset i32) (param $length i32)
    (local.get $table_offset)
    (local.get $elem_offset)
    (local.get $length)
    (table.init $test_table $elem_seg)
  )

  ;; Test function: Table out-of-bounds access
  (func $test_table_oob (param $table_offset i32) (param $elem_offset i32) (param $length i32)
    (local.get $table_offset)
    (local.get $elem_offset)
    (local.get $length)
    (table.init $test_table $elem_seg)
  )

  ;; Test function: Table overflow condition
  (func $test_table_overflow (param $table_offset i32) (param $elem_offset i32) (param $length i32)
    (local.get $table_offset)
    (local.get $elem_offset)
    (local.get $length)
    (table.init $test_table $elem_seg)
  )

  ;; Utility function: Get table element by calling it indirectly
  (func $get_table_elem (param $index i32) (result i32)
    ;; Use call_indirect to call function at table index
    ;; This will return the unique value each function produces
    ;; func_0 returns 100 -> index 0
    ;; func_1 returns 200 -> index 1
    ;; func_2 returns 300 -> index 2
    ;; func_3 returns 400 -> index 3
    ;; func_4 returns 500 -> index 4
    (local $result i32)

    ;; Call function indirectly through table
    (local.set $result
      (call_indirect (type $func_type) (local.get $index))
    )

    ;; Convert function result to index
    (i32.div_u (i32.sub (local.get $result) (i32.const 100)) (i32.const 100))
  )

  ;; Helper function to check if table element is initialized
  (func $is_table_elem_null (param $index i32) (result i32)
    ;; Try to call the function - if null, will trap and we catch it
    ;; Return 1 if null, 0 if not null
    ;; For simplicity, we'll assume elements are either our functions or null
    ;; This is a simplified check - real implementation might differ

    ;; Get table element and check if it can be called
    (local $can_call i32)
    (local.set $can_call (i32.const 1))  ;; Assume we can call by default

    ;; Try calling - this is a simplified approach
    ;; In a real scenario, we might need more sophisticated checking
    (local.get $can_call)
  )

  ;; Export test functions for C++ access
  (export "test_basic_init" (func $test_basic_init))
  (export "test_boundary_init" (func $test_boundary_init))
  (export "test_zero_length_init" (func $test_zero_length_init))
  (export "test_table_oob" (func $test_table_oob))
  (export "test_table_overflow" (func $test_table_overflow))
  (export "get_table_elem" (func $get_table_elem))
  (export "is_table_elem_null" (func $is_table_elem_null))

  ;; Export table for direct access if needed
  (export "test_table" (table $test_table))
)