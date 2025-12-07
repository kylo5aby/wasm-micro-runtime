(module
  ;; Immutable global - cannot be modified with global.set
  (global $g_immutable i32 (i32.const 42))

  ;; Function that attempts to set an immutable global
  ;; This should cause validation failure during module loading
  (func $set_immutable_global (result i32)
    i32.const 100
    global.set $g_immutable  ;; Invalid operation - global is immutable
    i32.const 1
  )

  ;; Export the function (though module should fail to load)
  (export "set_immutable_global" (func $set_immutable_global))
)