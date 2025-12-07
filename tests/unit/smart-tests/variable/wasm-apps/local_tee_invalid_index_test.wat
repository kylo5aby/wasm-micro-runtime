(module
  ;; Invalid local.tee test - should fail module validation
  ;; This module attempts to use local.tee with an invalid local variable index

  ;; Function with invalid local.tee index
  ;; This should cause module validation to fail
  (func $invalid_tee (export "invalid_tee") (param $input i32) (result i32)
    ;; No local variables declared, so index 1 is invalid (only param 0 exists)
    local.get $input
    local.tee 1  ;; ERROR: Invalid local index - should cause validation failure
  )
)