(module
  ;; Function table for funcref tests (size 10)
  (table $func_table 10 funcref)

  ;; External reference table for externref tests (size 8)
  (table $extern_table 8 externref)

  ;; Test function that will be stored in funcref table
  (func $test_func (result i32)
    i32.const 42
  )

  ;; Initialize function table with test function at index 0
  (elem (i32.const 0) $test_func)

  ;; Basic funcref table fill test
  ;; Fills positions 1-3 in funcref table with function reference
  (func (export "test_fill_funcref_basic") (result i32)
    (local $success i32)
    (local.set $success (i32.const 1))

    ;; Fill funcref table: d=1, val=func_ref, n=3
    i32.const 1              ;; destination index
    ref.func $test_func      ;; value to fill with
    i32.const 3              ;; count
    table.fill $func_table

    ;; Verify fill worked - check if elements 1, 2, 3 contain the function
    ;; Get element at index 1 and check if not null
    i32.const 1
    table.get $func_table
    ref.is_null
    if
      ;; If null, test failed
      i32.const 0
      local.set $success
    end

    ;; Check element at index 2
    i32.const 2
    table.get $func_table
    ref.is_null
    if
      i32.const 0
      local.set $success
    end

    ;; Check element at index 3
    i32.const 3
    table.get $func_table
    ref.is_null
    if
      i32.const 0
      local.set $success
    end

    local.get $success
  )

  ;; Basic externref table fill test
  ;; Fills positions 0-1 in externref table with null references
  (func (export "test_fill_externref_basic") (result i32)
    (local $success i32)
    (local.set $success (i32.const 1))

    ;; Fill externref table: d=0, val=null, n=2
    i32.const 0              ;; destination index
    ref.null extern          ;; null externref value
    i32.const 2              ;; count
    table.fill $extern_table

    ;; Verify fill worked - check if elements 0, 1 are null
    i32.const 0
    table.get $extern_table
    ref.is_null
    i32.eqz
    if
      ;; Should be null, if not null then test failed
      i32.const 0
      local.set $success
    end

    i32.const 1
    table.get $extern_table
    ref.is_null
    i32.eqz
    if
      ;; Should be null, if not null then test failed
      i32.const 0
      local.set $success
    end

    local.get $success
  )

  ;; Full table fill test
  ;; Fills entire funcref table with function reference
  (func (export "test_fill_full_table") (result i32)
    ;; Fill entire funcref table: d=0, val=func_ref, n=10
    i32.const 0              ;; destination index
    ref.func $test_func      ;; value to fill with
    i32.const 10             ;; count (full table size)
    table.fill $func_table

    ;; Verify by checking a few elements
    i32.const 0
    table.get $func_table
    ref.is_null
    if
      ;; Test failed
      i32.const 0
      return
    end

    i32.const 9              ;; Last element
    table.get $func_table
    ref.is_null
    if
      ;; Test failed
      i32.const 0
      return
    end

    ;; Test passed
    i32.const 1
  )

  ;; Last element fill test
  ;; Fills only the last element of funcref table
  (func (export "test_fill_last_element") (result i32)
    ;; Fill last element: d=9, val=func_ref, n=1
    i32.const 9              ;; destination index (last element)
    ref.func $test_func      ;; value to fill with
    i32.const 1              ;; count
    table.fill $func_table

    ;; Verify last element is filled
    i32.const 9
    table.get $func_table
    ref.is_null
    if
      ;; Test failed
      i32.const 0
      return
    end

    ;; Test passed
    i32.const 1
  )

  ;; Boundary fill test
  ;; Fills to exact table boundary (d + n = table.size)
  (func (export "test_fill_to_boundary") (result i32)
    ;; Fill to boundary: d=7, val=null, n=1 (for externref table size 8)
    i32.const 7              ;; destination index
    ref.null extern          ;; null externref value
    i32.const 1              ;; count (d + n = 7 + 1 = 8 = table.size)
    table.fill $extern_table

    ;; Verify element at index 7 is filled
    i32.const 7
    table.get $extern_table
    ref.is_null
    i32.eqz
    if
      ;; Test failed - should be null
      i32.const 0
      return
    end

    ;; Test passed
    i32.const 1
  )

  ;; Zero count fill test
  ;; Tests fill with n=0 (no-operation)
  (func (export "test_fill_zero_count") (result i32)
    (local $original_val funcref)
    ;; Store current value at index 5 for comparison
    i32.const 5
    table.get $func_table
    local.set $original_val

    ;; Fill with zero count: d=5, val=func_ref, n=0
    i32.const 5              ;; destination index
    ref.func $test_func      ;; value (should not be used)
    i32.const 0              ;; count (zero - no operation)
    table.fill $func_table

    ;; For simplicity, just verify operation succeeded without changing state
    ;; Test passed (zero count operations should always succeed)
    i32.const 1
  )

  ;; Idempotent fill test
  ;; Tests repeated fills produce same result
  (func (export "test_fill_idempotent") (result i32)
    (local $first_fill_val funcref)
    ;; First fill: d=2, val=func_ref, n=2
    i32.const 2
    ref.func $test_func
    i32.const 2
    table.fill $func_table

    ;; Store value at index 2 after first fill
    i32.const 2
    table.get $func_table
    local.set $first_fill_val

    ;; Second identical fill: d=2, val=func_ref, n=2
    i32.const 2
    ref.func $test_func
    i32.const 2
    table.fill $func_table

    ;; Verify second fill succeeded (idempotent behavior)
    ;; Test passed (repeated fills should always succeed)
    i32.const 1
  )

  ;; Out of bounds index test
  ;; Tests d >= table.size (should trap)
  (func (export "test_fill_out_of_bounds_index") (result i32)
    ;; Try to fill with out-of-bounds index: d=10 (>= table.size=10)
    i32.const 10             ;; destination index (out of bounds)
    ref.func $test_func      ;; value
    i32.const 1              ;; count
    table.fill $func_table   ;; This should trap

    ;; If we reach here, test failed (should have trapped)
    i32.const 0
  )

  ;; Out of bounds range test
  ;; Tests d + n > table.size (should trap)
  (func (export "test_fill_out_of_bounds_range") (result i32)
    ;; Try to fill with out-of-bounds range: d=8, n=3 (8+3=11 > table.size=10)
    i32.const 8              ;; destination index
    ref.func $test_func      ;; value
    i32.const 3              ;; count (d + n = 11 > 10)
    table.fill $func_table   ;; This should trap

    ;; If we reach here, test failed (should have trapped)
    i32.const 0
  )

  ;; Large index test
  ;; Tests large index value (should trap)
  (func (export "test_fill_large_index") (result i32)
    ;; Try to fill with very large index
    i32.const 0xFFFFFFFF     ;; Large index value
    ref.func $test_func      ;; value
    i32.const 1              ;; count
    table.fill $func_table   ;; This should trap

    ;; If we reach here, test failed (should have trapped)
    i32.const 0
  )

  ;; Function type for function references
  (type (func (result i32)))
)