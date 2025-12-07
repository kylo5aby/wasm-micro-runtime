(module
  ;; i32.xor stack underflow test module
  ;; This module contains valid WASM but represents scenarios that could
  ;; lead to stack underflow in more complex execution contexts

  ;; Function that performs valid i32.xor operation for validation
  ;; This represents a minimal valid case for comparison
  (func $valid_xor (param $a i32) (param $b i32) (result i32)
    local.get $a
    local.get $b
    i32.xor
  )

  ;; Function that uses conditional logic that might lead to stack issues
  ;; in dynamic execution scenarios - validates module loading capabilities
  (func $conditional_xor (param $condition i32) (param $a i32) (param $b i32) (result i32)
    local.get $condition
    if (result i32)
      local.get $a
      local.get $b
      i32.xor
    else
      i32.const 0
    end
  )

  ;; Export functions for testing
  (export "valid_xor" (func $valid_xor))
  (export "conditional_xor" (func $conditional_xor))
)