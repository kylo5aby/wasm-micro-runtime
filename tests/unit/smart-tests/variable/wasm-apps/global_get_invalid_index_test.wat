(module
  ;; Module with only one global but attempting to access invalid index
  (global $g_valid i32 (i32.const 42))

  ;; Function attempting to access invalid global index 1 (only index 0 exists)
  (func $get_invalid_global (result i32)
    global.get 1  ;; Invalid: only global index 0 exists
  )

  (export "get_invalid_global" (func $get_invalid_global))
)