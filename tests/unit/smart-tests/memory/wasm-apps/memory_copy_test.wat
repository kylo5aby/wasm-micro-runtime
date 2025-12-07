;; Enhanced WebAssembly Test Module for memory.copy Opcode
;;
;; This module provides comprehensive test functions for validating memory.copy
;; operations across different scenarios including basic copies, overlapping
;; regions, boundary conditions, and error cases.
;;
;; Memory Layout:
;; - Memory size: 2 pages (128KB) with room for growth testing
;; - Test regions distributed across memory to avoid interference
;; - Pattern-based data initialization for reliable validation
;;
;; Test Functions:
;; - test_memory_copy: Basic memory.copy wrapper function
;; - test_memory_copy_overlap: Overlapping region copy testing
;; - test_memory_copy_boundary: Boundary condition testing
;; - test_memory_copy_error: Error condition testing

(module
  ;; Memory definition: 2 pages (128KB) initial, 4 pages (256KB) maximum
  (memory $mem 2 4)

  ;; Export memory for external access and validation
  (export "memory" (memory $mem))

  ;; Basic memory.copy test function
  ;; Parameters: dest (i32), src (i32), size (i32)
  ;; Returns: none (operation success indicated by no trap)
  (func $test_memory_copy (param $dest i32) (param $src i32) (param $size i32)
    (memory.copy
      (local.get $dest)    ;; destination offset
      (local.get $src)     ;; source offset
      (local.get $size)    ;; number of bytes to copy
    )
  )
  (export "test_memory_copy" (func $test_memory_copy))

  ;; Memory copy with validation - returns 1 on success, 0 on failure
  ;; Parameters: dest (i32), src (i32), size (i32)
  ;; Returns: success flag (i32)
  (func $test_memory_copy_with_validation (param $dest i32) (param $src i32) (param $size i32) (result i32)
    (local $memory_size i32)

    ;; Get current memory size in bytes
    (local.set $memory_size
      (i32.mul
        (memory.size)
        (i32.const 65536)  ;; 64KB per page
      )
    )

    ;; Validate destination bounds
    (if (i32.gt_u
          (i32.add (local.get $dest) (local.get $size))
          (local.get $memory_size))
      (then
        (return (i32.const 0))  ;; Return failure
      )
    )

    ;; Validate source bounds
    (if (i32.gt_u
          (i32.add (local.get $src) (local.get $size))
          (local.get $memory_size))
      (then
        (return (i32.const 0))  ;; Return failure
      )
    )

    ;; Perform the copy
    (memory.copy
      (local.get $dest)
      (local.get $src)
      (local.get $size)
    )

    ;; Return success
    (i32.const 1)
  )
  (export "test_memory_copy_with_validation" (func $test_memory_copy_with_validation))

  ;; Overlapping memory copy test
  ;; Tests memory.copy behavior with overlapping source and destination
  ;; Parameters: dest (i32), src (i32), size (i32)
  ;; Returns: none
  (func $test_overlapping_copy (param $dest i32) (param $src i32) (param $size i32)
    ;; Direct memory.copy - WAMR should handle overlapping regions correctly
    (memory.copy
      (local.get $dest)
      (local.get $src)
      (local.get $size)
    )
  )
  (export "test_overlapping_copy" (func $test_overlapping_copy))

  ;; Zero-size copy test
  ;; Tests memory.copy with size=0 (should be no-op)
  ;; Parameters: dest (i32), src (i32)
  ;; Returns: none
  (func $test_zero_size_copy (param $dest i32) (param $src i32)
    (memory.copy
      (local.get $dest)
      (local.get $src)
      (i32.const 0)      ;; zero size - should be no-op
    )
  )
  (export "test_zero_size_copy" (func $test_zero_size_copy))

  ;; Memory initialization helper function
  ;; Fills memory region with specified pattern for testing
  ;; Parameters: offset (i32), size (i32), pattern (i32 - low byte used)
  ;; Returns: none
  (func $init_memory_pattern (param $offset i32) (param $size i32) (param $pattern i32)
    (local $i i32)
    (local $end i32)

    (local.set $end (i32.add (local.get $offset) (local.get $size)))
    (local.set $i (local.get $offset))

    (loop $fill_loop
      (i32.store8
        (local.get $i)
        (local.get $pattern)
      )

      (local.set $i (i32.add (local.get $i) (i32.const 1)))

      (br_if $fill_loop
        (i32.lt_u (local.get $i) (local.get $end))
      )
    )
  )
  (export "init_memory_pattern" (func $init_memory_pattern))

  ;; Memory verification helper function
  ;; Verifies memory region contains expected pattern
  ;; Parameters: offset (i32), size (i32), expected (i32 - low byte used)
  ;; Returns: 1 if all bytes match, 0 if mismatch found
  (func $verify_memory_pattern (param $offset i32) (param $size i32) (param $expected i32) (result i32)
    (local $i i32)
    (local $end i32)
    (local $actual i32)

    (local.set $end (i32.add (local.get $offset) (local.get $size)))
    (local.set $i (local.get $offset))

    (loop $verify_loop
      (local.set $actual (i32.load8_u (local.get $i)))

      ;; Check if current byte matches expected pattern
      (if (i32.ne (local.get $actual) (i32.and (local.get $expected) (i32.const 0xFF)))
        (then
          (return (i32.const 0))  ;; Mismatch found
        )
      )

      (local.set $i (i32.add (local.get $i) (i32.const 1)))

      (br_if $verify_loop
        (i32.lt_u (local.get $i) (local.get $end))
      )
    )

    ;; All bytes matched
    (i32.const 1)
  )
  (export "verify_memory_pattern" (func $verify_memory_pattern))

  ;; Large memory copy test
  ;; Tests memory.copy with large size parameters
  ;; Parameters: dest (i32), src (i32), size (i32)
  ;; Returns: none
  (func $test_large_copy (param $dest i32) (param $src i32) (param $size i32)
    ;; Ensure we have enough memory for large copy
    (if (i32.gt_u
          (i32.add (local.get $dest) (local.get $size))
          (i32.mul (memory.size) (i32.const 65536)))
      (then
        ;; Grow memory if needed (this may trap if growth fails)
        (drop (memory.grow (i32.const 1)))
      )
    )

    ;; Perform large copy
    (memory.copy
      (local.get $dest)
      (local.get $src)
      (local.get $size)
    )
  )
  (export "test_large_copy" (func $test_large_copy))

  ;; Boundary copy test
  ;; Tests memory.copy at memory boundaries (start and near end)
  ;; Parameters: test_type (i32) - 0 for start, 1 for end boundary
  ;; Returns: none
  (func $test_boundary_copy (param $test_type i32)
    (if (i32.eqz (local.get $test_type))
      (then
        ;; Test copy to memory start (offset 0)
        (memory.copy
          (i32.const 0)     ;; destination at memory start
          (i32.const 64)    ;; source at safe offset
          (i32.const 32)    ;; copy 32 bytes
        )
      )
      (else
        ;; Test copy near memory end
        (memory.copy
          (i32.sub              ;; destination near end
            (i32.mul (memory.size) (i32.const 65536))
            (i32.const 64)
          )
          (i32.sub              ;; source before destination
            (i32.mul (memory.size) (i32.const 65536))
            (i32.const 128)
          )
          (i32.const 32)        ;; copy 32 bytes
        )
      )
    )
  )
  (export "test_boundary_copy" (func $test_boundary_copy))

  ;; Error condition test - out of bounds copy
  ;; This function is designed to trap when called with out-of-bounds parameters
  ;; Parameters: dest (i32), src (i32), size (i32)
  ;; Returns: none (expected to trap)
  (func $test_out_of_bounds_copy (param $dest i32) (param $src i32) (param $size i32)
    ;; This will trap if any parameter causes out-of-bounds access
    (memory.copy
      (local.get $dest)
      (local.get $src)
      (local.get $size)
    )
  )
  (export "test_out_of_bounds_copy" (func $test_out_of_bounds_copy))

  ;; Self-copy test (source == destination)
  ;; Tests memory.copy when source and destination are identical
  ;; Parameters: offset (i32), size (i32)
  ;; Returns: none
  (func $test_self_copy (param $offset i32) (param $size i32)
    (memory.copy
      (local.get $offset)   ;; destination
      (local.get $offset)   ;; source (same as destination)
      (local.get $size)
    )
  )
  (export "test_self_copy" (func $test_self_copy))

  ;; Multi-step copy test
  ;; Tests sequential memory.copy operations for transitivity
  ;; Parameters: step1_dest (i32), step1_src (i32), step2_dest (i32), size (i32)
  ;; Returns: none
  (func $test_multi_step_copy (param $step1_dest i32) (param $step1_src i32)
                              (param $step2_dest i32) (param $size i32)
    ;; First copy: step1_src -> step1_dest
    (memory.copy
      (local.get $step1_dest)
      (local.get $step1_src)
      (local.get $size)
    )

    ;; Second copy: step1_dest -> step2_dest
    (memory.copy
      (local.get $step2_dest)
      (local.get $step1_dest)
      (local.get $size)
    )
  )
  (export "test_multi_step_copy" (func $test_multi_step_copy))

  ;; Memory size query function
  ;; Returns current memory size in bytes
  ;; Returns: memory size (i32)
  (func $get_memory_size (result i32)
    (i32.mul
      (memory.size)
      (i32.const 65536)  ;; Convert pages to bytes
    )
  )
  (export "get_memory_size" (func $get_memory_size))
)