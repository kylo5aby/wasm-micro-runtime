(module
  ;; Define two funcref tables for testing table.copy operations
  (table $table0 10 funcref)
  (table $table1 10 funcref)

  ;; Helper functions to be referenced in tables
  (func $helper_func1 (result i32) i32.const 1)
  (func $helper_func2 (result i32) i32.const 2)
  (func $helper_func3 (result i32) i32.const 3)

  ;; Element section to make functions available for table operations
  (elem declare func $helper_func1 $helper_func2 $helper_func3)

  ;; Test basic table copy functionality
  (func (export "test_basic_table_copy") (param $dummy i32) (result i32)
    ;; Initialize table0 with some functions
    i32.const 0
    ref.func $helper_func1
    table.set $table0

    i32.const 1
    ref.func $helper_func2
    table.set $table0

    i32.const 2
    ref.func $helper_func3
    table.set $table0

    ;; Copy 3 elements from table0[0-2] to table1[1-3]
    i32.const 1    ;; dst_offset
    i32.const 0    ;; src_offset
    i32.const 3    ;; len
    table.copy $table1 $table0

    ;; Verify the copy by checking that table1[1] is not null
    i32.const 1
    table.get $table1
    ref.is_null

    ;; Return 1 if verification passes (element is not null), 0 if fails
    i32.eqz
  )

  ;; Test overlapping table copy within same table
  (func (export "test_overlap_table_copy") (param $dummy i32) (result i32)
    ;; Initialize table0 with sequential functions
    i32.const 0
    ref.func $helper_func1
    table.set $table0

    i32.const 1
    ref.func $helper_func2
    table.set $table0

    i32.const 2
    ref.func $helper_func3
    table.set $table0

    ;; Copy elements within same table (overlapping)
    ;; Copy table0[0-1] to table0[1-2]
    i32.const 1    ;; dst_offset
    i32.const 0    ;; src_offset
    i32.const 2    ;; len
    table.copy $table0 $table0

    ;; Verify by checking that table0[1] is not null
    i32.const 1
    table.get $table0
    ref.is_null

    ;; Return 1 if verification passes, 0 if fails
    i32.eqz
  )

  ;; Test boundary conditions
  (func (export "test_boundary_table_copy") (param $dummy i32) (result i32)
    ;; Test zero-length copy (should be no-op)
    i32.const 5    ;; dst_offset
    i32.const 5    ;; src_offset
    i32.const 0    ;; len (zero length)
    table.copy $table0 $table0

    ;; Test copy at table boundary
    ;; Initialize last element
    i32.const 9    ;; last position
    ref.func $helper_func1
    table.set $table0

    ;; Copy last element to first position
    i32.const 0    ;; dst_offset
    i32.const 9    ;; src_offset
    i32.const 1    ;; len
    table.copy $table0 $table0

    ;; Verify copy worked
    i32.const 0
    table.get $table0
    ref.is_null

    ;; Return 1 if verification passes, 0 if fails
    i32.eqz
  )

  ;; Test error conditions that should trap
  (func (export "test_error_table_copy") (param $test_case i32) (result i32)
    (if (result i32) (i32.eq (local.get $test_case) (i32.const 1)) (then
      ;; Test case 1: Out-of-bounds destination offset
      i32.const 10   ;; dst_offset (out of bounds)
      i32.const 0    ;; src_offset
      i32.const 1    ;; len
      table.copy $table0 $table0
      i32.const 1    ;; Should not reach here
    ) (else
      (if (result i32) (i32.eq (local.get $test_case) (i32.const 2)) (then
        ;; Test case 2: Out-of-bounds source offset
        i32.const 0    ;; dst_offset
        i32.const 10   ;; src_offset (out of bounds)
        i32.const 1    ;; len
        table.copy $table0 $table0
        i32.const 1    ;; Should not reach here
      ) (else
        (if (result i32) (i32.eq (local.get $test_case) (i32.const 3)) (then
          ;; Test case 3: Length overflow on destination
          i32.const 9    ;; dst_offset
          i32.const 0    ;; src_offset
          i32.const 2    ;; len (9 + 2 > 10)
          table.copy $table0 $table0
          i32.const 1    ;; Should not reach here
        ) (else
          (if (result i32) (i32.eq (local.get $test_case) (i32.const 4)) (then
            ;; Test case 4: Length overflow on source
            i32.const 0    ;; dst_offset
            i32.const 9    ;; src_offset
            i32.const 2    ;; len (9 + 2 > 10)
            table.copy $table0 $table0
            i32.const 1    ;; Should not reach here
          ) (else
            ;; Unknown test case
            i32.const 0
          ))
        ))
      ))
    ))
  )
)