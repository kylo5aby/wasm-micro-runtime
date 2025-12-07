(module
  ;; Single mutable global for testing
  (global $g_valid (mut i32) (i32.const 0))

  ;; Function that attempts to set an invalid global index
  ;; This should cause validation failure during module loading
  ;; Invalid global index: 99 (only index 0 exists)
  (func $set_invalid_global_index (result i32)
    i32.const 42
    global.set 99  ;; Invalid index - should cause module validation failure
    i32.const 1
  )

  ;; Export the function (though module should fail to load)
  (export "set_invalid_global_index" (func $set_invalid_global_index))
)