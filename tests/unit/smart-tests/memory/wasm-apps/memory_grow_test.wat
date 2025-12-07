(module
  ;; Basic memory configuration: 2 pages initial, 10 pages maximum
  ;; This provides reasonable space for testing successful growth operations
  (memory (export "memory") 2 10)

  ;; Function to get current memory size using memory.size instruction
  (func (export "get_memory_size") (result i32)
    memory.size
  )

  ;; Function to grow memory by specified pages using memory.grow instruction
  ;; Returns the previous memory size (before growth) or -1 if growth fails
  (func (export "grow_memory") (param $pages i32) (result i32)
    (local.get $pages)
    memory.grow
  )

  ;; Function to grow memory and then return the new size for validation
  ;; This is useful for testing integration between memory.grow and memory.size
  (func (export "grow_and_get_new_size") (param $pages i32) (result i32)
    (local $growth_result i32)

    ;; Attempt to grow memory
    (local.set $growth_result (memory.grow (local.get $pages)))

    ;; Return new memory size (for successful growth validation)
    memory.size
  )

  ;; Function to perform consecutive growth operations for sequence testing
  ;; Grows by 1 page, then by 2 pages, then by 1 page again
  ;; Returns array-like results through memory for validation
  (func (export "consecutive_growth_test") (result i32)
    (local $result1 i32)
    (local $result2 i32)
    (local $result3 i32)

    ;; First growth: grow by 1 page
    (local.set $result1 (memory.grow (i32.const 1)))

    ;; Second growth: grow by 2 pages
    (local.set $result2 (memory.grow (i32.const 2)))

    ;; Third growth: grow by 1 page
    (local.set $result3 (memory.grow (i32.const 1)))

    ;; Return the final memory size for validation
    memory.size
  )

  ;; Function to test zero growth (identity operation)
  ;; Should return current memory size without changing memory state
  (func (export "zero_growth_test") (result i32)
    (memory.grow (i32.const 0))
  )

  ;; Function to test large growth request that should fail
  ;; Attempts to grow by maximum i32 value (0x7FFFFFFF)
  (func (export "large_growth_test") (result i32)
    (memory.grow (i32.const 0x7FFFFFFF))
  )

  ;; Function to validate memory state before and after operations
  ;; Returns 1 if memory state is consistent, 0 otherwise
  (func (export "validate_memory_consistency") (param $expected_size i32) (result i32)
    (local $current_size i32)
    (local.set $current_size (memory.size))

    ;; Compare current size with expected size
    (if (result i32) (i32.eq (local.get $current_size) (local.get $expected_size))
      (then (i32.const 1))  ;; Consistent
      (else (i32.const 0))  ;; Inconsistent
    )
  )
)