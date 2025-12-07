(module
  ;; Limited memory configuration: 1 page initial, 3 pages maximum
  ;; This configuration is designed for boundary and failure testing
  (memory (export "memory") 1 3)

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

  ;; Function to test growth to exact memory limit
  ;; Attempts to grow from 1 page to 3 pages (maximum allowed)
  (func (export "grow_to_limit") (result i32)
    (memory.grow (i32.const 2))  ;; 1 + 2 = 3 pages (exactly at limit)
  )

  ;; Function to test growth beyond memory limit (should fail)
  ;; Attempts to grow from 1 page by 3 pages (would need 4 total, exceeds max 3)
  (func (export "grow_beyond_limit") (result i32)
    (memory.grow (i32.const 3))  ;; 1 + 3 = 4 pages (exceeds limit of 3)
  )

  ;; Function to test consecutive failed growth attempts
  ;; Tests multiple failure scenarios to ensure consistent behavior
  (func (export "multiple_failed_growths") (result i32)
    (local $result1 i32)
    (local $result2 i32)
    (local $result3 i32)
    (local $final_size i32)

    ;; Attempt 1: Try to grow by 100 pages (should fail)
    (local.set $result1 (memory.grow (i32.const 100)))

    ;; Attempt 2: Try to grow by 1000 pages (should fail)
    (local.set $result2 (memory.grow (i32.const 1000)))

    ;; Attempt 3: Try to grow by maximum i32 value (should fail)
    (local.set $result3 (memory.grow (i32.const 0x7FFFFFFF)))

    ;; Check that all attempts failed and return current size
    ;; All results should be -1, memory should remain at initial size
    (local.set $final_size (memory.size))

    ;; Validate that all growth attempts failed (-1) and memory unchanged
    (if (result i32) (i32.and
                       (i32.and
                         (i32.eq (local.get $result1) (i32.const -1))
                         (i32.eq (local.get $result2) (i32.const -1))
                       )
                       (i32.and
                         (i32.eq (local.get $result3) (i32.const -1))
                         (i32.eq (local.get $final_size) (i32.const 1))
                       )
                     )
      (then (i32.const 1))  ;; All tests passed
      (else (i32.const 0))  ;; At least one test failed
    )
  )

  ;; Function to test growth at boundary conditions
  ;; Tests the boundary between successful and failed growth
  (func (export "boundary_growth_test") (result i32)
    (local $growth1 i32)
    (local $growth2 i32)
    (local $size_after_success i32)
    (local $size_after_failure i32)

    ;; First: Grow to exactly the limit (should succeed)
    (local.set $growth1 (memory.grow (i32.const 2)))  ;; 1 + 2 = 3 (success)

    ;; Check size after successful growth
    (local.set $size_after_success (memory.size))

    ;; Second: Try to grow further (should fail as we're at limit)
    (local.set $growth2 (memory.grow (i32.const 1)))  ;; 3 + 1 = 4 (exceeds limit)

    ;; Check size after failed growth (should remain unchanged)
    (local.set $size_after_failure (memory.size))

    ;; Validate the boundary behavior:
    ;; - First growth should return 1 (previous size)
    ;; - Size after success should be 3 (at limit)
    ;; - Second growth should return -1 (failure)
    ;; - Size after failure should still be 3 (unchanged)
    (if (result i32) (i32.and
                       (i32.and
                         (i32.eq (local.get $growth1) (i32.const 1))
                         (i32.eq (local.get $size_after_success) (i32.const 3))
                       )
                       (i32.and
                         (i32.eq (local.get $growth2) (i32.const -1))
                         (i32.eq (local.get $size_after_failure) (i32.const 3))
                       )
                     )
      (then (i32.const 1))  ;; Boundary test passed
      (else (i32.const 0))  ;; Boundary test failed
    )
  )

  ;; Function to test zero growth at limit
  ;; Tests identity operation when memory is at maximum
  (func (export "zero_growth_at_limit") (result i32)
    (local $grow_to_limit i32)
    (local $zero_growth i32)

    ;; First grow to limit
    (local.set $grow_to_limit (memory.grow (i32.const 2)))

    ;; Then perform zero growth (should return current size)
    (local.set $zero_growth (memory.grow (i32.const 0)))

    ;; Validate: first growth should succeed (return 1), zero growth should return 3
    (if (result i32) (i32.and
                       (i32.eq (local.get $grow_to_limit) (i32.const 1))
                       (i32.eq (local.get $zero_growth) (i32.const 3))
                     )
      (then (i32.const 1))  ;; Zero growth at limit test passed
      (else (i32.const 0))  ;; Zero growth at limit test failed
    )
  )
)