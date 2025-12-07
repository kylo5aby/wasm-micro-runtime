(module
  ;; This module contains an invalid block type reference
  ;; The block references type index 99 which doesn't exist in the type section

  ;; Type section with only one type (index 0)
  (type $valid_type (func (result i32)))

  ;; Function that uses invalid block type index 99
  (func (export "invalid_block_function") (result i32)
    block (type 99)  ;; Invalid type index - should cause module loading to fail
      i32.const 42
    end
  )
)