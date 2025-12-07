(module
  ;; Invalid function that attempts i8x16.splat without sufficient stack values
  ;; This should fail during module validation due to stack underflow
  (func $invalid_i8x16_splat (result v128)
    ;; Attempting i8x16.splat without providing i32 parameter
    ;; This creates a stack underflow scenario that should be caught
    ;; during WASM module validation
    i8x16.splat  ;; ERROR: No i32 value on stack
  )
  (export "invalid_i8x16_splat" (func $invalid_i8x16_splat))
)