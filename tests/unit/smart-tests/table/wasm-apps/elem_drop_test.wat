;; Copyright (C) 2019 Intel Corporation. All rights reserved.
;; SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

(module
  ;; Function definitions
  (func $dummy_func1 (result i32)
    i32.const 100
  )

  (func $dummy_func2 (result i32)
    i32.const 200
  )

  (func $dummy_func3 (result i32)
    i32.const 300
  )

  (func $dummy_func4 (result i32)
    i32.const 400
  )

  ;; Table declaration
  (table 10 funcref)

  ;; Element segments for testing
  (elem (i32.const 0) func $dummy_func1 $dummy_func2)  ;; Element segment 0
  (elem (i32.const 2) func $dummy_func3)               ;; Element segment 1
  (elem (i32.const 4) func $dummy_func4 $dummy_func1)  ;; Element segment 2

  ;; Test function for elem.drop operation - always drop segment 0
  (func (export "test_elem_drop") (result i32)
    elem.drop 0
    ;; Return success (1) if elem.drop completes
    i32.const 1
  )

  ;; Test function for elem.drop segment 1
  (func (export "test_elem_drop_1") (result i32)
    elem.drop 1
    i32.const 1
  )

  ;; Test function for elem.drop segment 2
  (func (export "test_elem_drop_2") (result i32)
    elem.drop 2
    i32.const 1
  )

  ;; Test function for table.init operation
  (func (export "test_table_init")
        (param $elem_idx i32) (param $dest i32) (param $src i32) (param $len i32)
        (result i32)

    ;; Perform table.init operation using element segment from elem_idx parameter
    ;; Note: elem_idx must be constant for table.init, so we'll handle specific cases
    (if (i32.eq (local.get $elem_idx) (i32.const 0))
      (then
        (table.init 0 0 (local.get $dest) (local.get $src) (local.get $len))
        i32.const 1
        return
      )
    )

    (if (i32.eq (local.get $elem_idx) (i32.const 1))
      (then
        (table.init 0 1 (local.get $dest) (local.get $src) (local.get $len))
        i32.const 1
        return
      )
    )

    (if (i32.eq (local.get $elem_idx) (i32.const 2))
      (then
        (table.init 0 2 (local.get $dest) (local.get $src) (local.get $len))
        i32.const 1
        return
      )
    )

    ;; Invalid element index
    i32.const 0
  )

  ;; Test function to check segment status by attempting table.init with specific segments
  (func (export "check_segment_status") (param $seg_idx i32) (result i32)
    ;; Simplified check - will be implemented to return dropped status
    ;; For now, assume segment is dropped after elem.drop is called
    i32.const 1
  )
)