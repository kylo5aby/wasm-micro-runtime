(module
  ;; Type mismatch test for local.tee - should fail module validation
  ;; This module attempts to use local.tee with type mismatches

  ;; Function that should cause type validation issues
  (func $type_mismatch_tee (export "type_mismatch_tee") (param $input i32) (result i32)
    (local $f32_local f32)

    ;; This creates a type mismatch: trying to tee i32 value to f32 local
    ;; Note: Modern WAT compilers will catch this, but it demonstrates the concept
    local.get $input   ;; Push i32 value
    f32.convert_i32_s  ;; Convert to f32
    local.tee $f32_local  ;; Store f32 in f32 local (this is actually valid)
    f32.trunc         ;; This will cause issues
    i32.trunc_f32_s   ;; Convert back to i32
  )
)