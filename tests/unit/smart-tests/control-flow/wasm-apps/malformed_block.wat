(module
  ;; This module contains a malformed block structure
  ;; The block is missing its matching 'end' instruction

  (func (export "malformed_block_function") (result i32)
    block (result i32)
      i32.const 42
    ;; Missing 'end' instruction - should cause validation failure
  )
)