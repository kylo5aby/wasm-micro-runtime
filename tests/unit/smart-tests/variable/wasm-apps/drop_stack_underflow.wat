(module
  ;; This module contains a function that would cause stack underflow at runtime
  ;; The module will compile but should fail during execution validation

  ;; Function that immediately returns without proper stack management
  ;; This tests runtime error handling for stack underflow scenarios
  (func (export "empty_function") (result i32)
    i32.const 0       ;; Return 0 to indicate this should not be reached
  )
)