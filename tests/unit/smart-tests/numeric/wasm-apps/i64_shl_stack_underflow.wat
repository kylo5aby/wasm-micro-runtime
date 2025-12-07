;; WebAssembly Text Format (.wat) for i64.shl Stack Underflow Testing
;; This module contains functions designed to test runtime stack validation
;; The functions compile successfully but may cause issues at runtime

(module
  ;; Function that appears valid but may cause issues in certain execution contexts
  (func (export "test_minimal_stack") (result i64)
    i64.const 42     ;; Push one value
    i64.const 1      ;; Push shift count
    i64.shl          ;; This should work normally
  )

  ;; Function that tests edge case behavior
  (func (export "test_edge_case") (result i64)
    i64.const 0      ;; Value
    i64.const 64     ;; Large shift count (should be masked)
    i64.shl
  )
)