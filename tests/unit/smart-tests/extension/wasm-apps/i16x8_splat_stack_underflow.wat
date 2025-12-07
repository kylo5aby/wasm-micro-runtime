(module
  ;; Invalid function that attempts i16x8.splat without sufficient stack values
  ;; This should fail during module validation due to stack underflow
  (func $invalid_i16x8_splat (result v128)
    ;; Attempting i16x8.splat without providing i32 parameter
    ;; This creates a stack underflow scenario that should be caught
    ;; during WASM module validation
    i16x8.splat  ;; ERROR: No i32 value on stack
  )
  (export "invalid_i16x8_splat" (func $invalid_i16x8_splat))
)