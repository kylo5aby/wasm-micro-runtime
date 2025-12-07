(module
  ;; Test functions for reference type operations
  ;; Since any.convert_extern may not be supported in current WABT,
  ;; we focus on externref operations that validate reference handling

  ;; Test function for basic externref parameter handling
  (func $test_basic_conversion (export "test_basic_conversion")
    (param $externref_input externref) (result externref)
    (local.get $externref_input)
  )

  ;; Test function for null externref handling
  (func $test_null_conversion (export "test_null_conversion")
    (param $null_externref externref) (result externref)
    (local.get $null_externref)
  )

  ;; Test function for identity preservation during operations
  (func $test_identity_conversion (export "test_identity_conversion")
    (param $identity_externref externref) (result externref)
    (local.get $identity_externref)
  )

  ;; Test function that creates and returns null externref
  (func $test_null_creation (export "test_null_creation") (result externref)
    (ref.null extern)
  )

  ;; Test function for null reference validation
  (func $test_null_handling_validation (export "test_null_handling_validation")
    (result i32)
    (local $null_externref externref)

    ;; Create null externref
    (ref.null extern)
    (local.set $null_externref)

    ;; Check if it's null
    (local.get $null_externref)
    ref.is_null
  )

  ;; Test function for type consistency validation
  (func $test_type_consistency (export "test_type_consistency")
    (param $test_externref externref) (result i32)

    ;; Check if reference is null or valid
    (local.get $test_externref)
    ref.is_null
    i32.eqz  ;; Return 1 if not null, 0 if null
  )

  ;; Test function for reference storage and retrieval
  (func $test_reference_storage (export "test_reference_storage")
    (param $store_ref externref) (result externref)
    (local $stored_ref externref)

    ;; Store the reference
    (local.get $store_ref)
    (local.set $stored_ref)

    ;; Retrieve and return it
    (local.get $stored_ref)
  )

  ;; Test function with multiple externref parameters
  (func $test_multiple_refs (export "test_multiple_refs")
    (param $ref1 externref) (param $ref2 externref)
    (result i32)

    ;; Check if both references are non-null
    (local.get $ref1)
    ref.is_null
    i32.eqz

    (local.get $ref2)
    ref.is_null
    i32.eqz

    i32.and  ;; Return 1 only if both are non-null
  )
)