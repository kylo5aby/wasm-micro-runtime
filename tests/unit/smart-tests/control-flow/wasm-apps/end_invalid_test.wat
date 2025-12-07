(module
  ;; Invalid module with type mismatch at block boundary
  (func $invalid_type_mismatch (result i32)
    (block (result f32)  ;; Block expects f32 but function returns i32
      f32.const 42.0
    )
    drop  ;; This should create a type mismatch error
    i32.const 0
  )
)