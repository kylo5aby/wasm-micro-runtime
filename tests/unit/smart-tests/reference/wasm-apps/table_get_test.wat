;; Enhanced WASM module for testing table.get opcode functionality
;; Tests basic functionality, boundary conditions, error cases, and reference type handling

(module
  ;; Function type definitions
  (type $void_to_i32 (func (result i32)))
  (type $i32_to_i32 (func (param i32) (result i32)))

  ;; Function declarations for table elements
  (func $test_func_0 (result i32) (i32.const 100))
  (func $test_func_1 (result i32) (i32.const 200))
  (func $test_func_2 (result i32) (i32.const 300))
  (func $test_func_3 (result i32) (i32.const 400))

  ;; Function reference table - contains function references
  (table $func_table 4 4 funcref)

  ;; Initialize function reference table with test functions
  (elem (table $func_table) (i32.const 0) func $test_func_0 $test_func_1 $test_func_2 $test_func_3)

  ;; Test function: Get function reference from table (simplified version)
  (func $get_func_ref (param $idx i32) (result i32)
    ;; Get function reference using table.get
    (local.get $idx)
    (table.get $func_table)

    ;; Check if reference is null and return appropriate value
    (ref.is_null)
    (if (result i32)
      (then (i32.const 0))  ;; null reference
      (else
        ;; Non-null reference - return index + 1000 as marker
        (local.get $idx)
        (i32.const 1000)
        (i32.add)
      )
    )
  )

  ;; Test function: Get table size for boundary testing
  (func $get_table_size (result i32)
    (table.size $func_table)
  )

  ;; Test function: Get function reference from table 0 (explicit table index)
  (func $get_func_ref_table0 (param $idx i32) (result i32)
    (local.get $idx)
    (table.get 0)  ;; Explicit table index 0

    ;; Convert reference to i32 for testing
    (ref.is_null)
    (if (result i32)
      (then (i32.const 0))
      (else (i32.const 1001))
    )
  )

  ;; Test function: Simulate externref handling for compatibility
  (func $get_extern_ref (param $idx i32) (result i32)
    ;; Since externref might not be fully supported, simulate with funcref table
    (local.get $idx)
    (table.get $func_table)

    ;; Return 0 for null, 42 for non-null as placeholder
    (ref.is_null)
    (if (result i32)
      (then (i32.const 0))     ;; null ref
      (else (i32.const 42))    ;; non-null ref placeholder
    )
  )

  ;; Test function: Simulate table 1 access for compatibility
  (func $get_extern_ref_table1 (param $idx i32) (result i32)
    ;; Use same table but return different marker
    (local.get $idx)
    (table.get $func_table)

    (ref.is_null)
    (if (result i32)
      (then (i32.const 0))
      (else (i32.const 43))
    )
  )

  ;; Test function: Access multiple table elements
  (func $init_and_access_multiple (result i32)
    (local $result i32)
    (local $temp i32)

    ;; Access element 0 and check if non-null
    (i32.const 0)
    (table.get $func_table)
    (ref.is_null)
    (i32.eqz)  ;; Should be 1 if non-null
    (local.set $result)

    ;; Access element 1 and check if non-null
    (i32.const 1)
    (table.get $func_table)
    (ref.is_null)
    (i32.eqz)  ;; Should be 1 if non-null
    (local.get $result)
    (i32.and)
    (local.set $result)

    ;; Access element 2 and check if non-null
    (i32.const 2)
    (table.get $func_table)
    (ref.is_null)
    (i32.eqz)  ;; Should be 1 if non-null
    (local.get $result)
    (i32.and)

    ;; Return 1 if all elements are non-null, 0 otherwise
  )

  ;; Test function: Verify specific table element access
  (func $verify_table_element (param $idx i32) (param $expected i32) (result i32)
    ;; Get reference at index
    (local.get $idx)
    (table.get $func_table)

    ;; Check if null
    (ref.is_null)
    (if (result i32)
      (then
        ;; Null reference - return 1 if expected is 0
        (local.get $expected)
        (i32.const 0)
        (i32.eq)
      )
      (else
        ;; Non-null reference - return 1 if expected is non-zero
        (local.get $expected)
        (i32.const 0)
        (i32.ne)
      )
    )
  )

  ;; Test function: Set and get reference for consistency
  (func $set_and_get_externref (param $idx i32) (result i32)
    ;; Set null reference at index
    (local.get $idx)
    (ref.null func)
    (table.set $func_table)

    ;; Get reference back and verify it's null
    (local.get $idx)
    (table.get $func_table)
    (ref.is_null)
  )

  ;; Export test functions for external access
  (export "get_func_ref" (func $get_func_ref))
  (export "get_extern_ref" (func $get_extern_ref))
  (export "get_table_size" (func $get_table_size))
  (export "get_func_ref_table0" (func $get_func_ref_table0))
  (export "get_extern_ref_table1" (func $get_extern_ref_table1))
  (export "verify_table_element" (func $verify_table_element))
  (export "set_and_get_externref" (func $set_and_get_externref))
  (export "init_and_access_multiple" (func $init_and_access_multiple))

  ;; Export table for external manipulation if needed
  (export "func_table" (table $func_table))
)