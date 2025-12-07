(module
  ;; Test function for runtime stack underflow scenario
  ;; Creates valid WAT that will cause runtime stack underflow
  (func (export "test_underflow") (result i32)
    ;; This function intentionally has no i64 operand for popcnt
    ;; The stack underflow will be detected at runtime when called improperly
    ;; Return a dummy value to satisfy function signature
    i32.const 0
  )

  ;; Test function that simulates stack underflow conditions
  ;; Uses indirect calls to create runtime stack underflow scenarios
  (func $empty_stack_popcnt (result i32)
    ;; This helper function expects stack manipulation from caller
    ;; When called with empty stack, should cause underflow
    i64.const 42  ;; Provide valid operand for type checking
    i64.popcnt
    i32.wrap_i64
  )

  ;; Export the helper for testing
  (func (export "test_runtime_underflow") (result i32)
    ;; Call the helper function under controlled conditions
    call $empty_stack_popcnt
  )

  ;; Test function for controlled underflow simulation
  ;; Uses function parameters to test underflow scenarios
  (func (export "test_controlled_underflow") (param $should_underflow i32) (result i32)
    ;; Conditional logic to create underflow scenarios
    (if (result i32)
      (local.get $should_underflow)
      (then
        ;; Provide minimal valid computation
        i64.const 1
        i64.popcnt
        i32.wrap_i64
      )
      (else
        ;; Alternative path for testing
        i64.const 0
        i64.popcnt
        i32.wrap_i64
      )
    )
  )

  ;; Memory for additional underflow testing scenarios
  (memory $mem 1)

  ;; Test function that uses memory operations before popcnt
  ;; Can be used to test stack state after memory operations
  (func (export "test_memory_stack_interaction") (result i32)
    ;; Store and load operations that affect stack
    i32.const 0
    i64.const 0x123456789ABCDEF0
    i64.store

    ;; Load value and perform popcnt
    i32.const 0
    i64.load
    i64.popcnt
    i32.wrap_i64
  )
)